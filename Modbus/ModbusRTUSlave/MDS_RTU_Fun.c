/********************************************************************************
* @File name: MD_RTU_Fun.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU Slave�ӻ����չ��ܺ�����
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MDS_RTU_Fun.h"
#include "MD_RTU_CRC16.h"
#include "MDS_RTU_Serial.h"
#include "MDS_RTU_User_Fun.h"
/*********************************����******************************************/

/**********************************��������*************************************/
void MDS_RTU_RecvByte(void *obj,uint8 byte);
void MDS_RTU_TimeHandler(void *obj);

static void MDS_RTU_SendErrorCode(PModbusS_RTU pModbus_RTU,ANLCode anlCode,ErrorCode errCode);
uint8 MDS_RTU_ReadDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,uint16 regNum,uint8 funCode);
uint8 MDS_RTU_WriteDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,uint16 regNum,uint8 funCode,uint16* data,uint8 byteCount);
/*********************************����******************************************/

/*******************************************************
*
* Function name :MDS_RTU_Init
* Description        :��ʼ��һ���ӻ�
* Parameter         :
*        @pModbusRTU            �ӻ��ṹ��ָ��
*        @mdRTUSerialInitFun    Ӳ�����ڳ�ʼ������  
*        @salveAddr            �ӻ���ַ
*        @baud            ������
*        @dataBits            ����λ
*        @stopBits           ֹͣλ 
*        @parity       ��żУ��λ
* Return          : ��
**********************************************************/
void MDS_RTU_Init(PModbusS_RTU pModbusRTU,MD_RTU_SerialInit mdRTUSerialInitFun,uint8 salveAddr,uint32 baud,uint8 dataBits,uint8 stopBits,uint8 parity){
	uint8 i;
	float T;
	if(pModbusRTU==NULL){
		return ;
	}
	MDInitQueue(&(pModbusRTU->mdSqQueue));
	MDInitQueue(&(pModbusRTU->mdMsgSqQueue));
	pModbusRTU->salveAddr=salveAddr;
	pModbusRTU->serialReadCount=0;
#if MDS_USE_SEND_CACHE
	pModbusRTU->serialSendCount=0;
#endif
	for(i=0;i<MDS_REG_COIL_ITEM_NUM;i++){
		pModbusRTU->pMapTableList[i] = NULL;
	}
	
	TO_MDBase(pModbusRTU)->mdRTUTimeHandlerFunction=MDS_RTU_TimeHandler;
	TO_MDBase(pModbusRTU)->mdRTURecByteFunction=MDS_RTU_RecvByte;
	TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction=MDSSerialSendBytes;
	TO_MDBase(pModbusRTU)->mdRTURecSendConv=MDSSerialSWRecv_Send;
	
	pModbusRTU->mdsWriteFun=NULL;
	pModbusRTU->lastTimesTick=0xFFFFFFFF;
	pModbusRTU->lastSendTimes=0x00000000;
	pModbusRTU->timesTick=0x00000000;
	
	T=(1.0/(float)baud)*100000;// 100us
	uint16 time=0;
	time=T*(dataBits+(parity?1:0));
	if(stopBits==0){
		time+=T;
	}else if(stopBits==1){
		time+=T*1.5f;
	}else if(stopBits==2){
		time+=T*2;
	}
	pModbusRTU->frameIntervalTime=time;/*�ò�����Ҫ���ݲ���������*/
	
	pModbusRTU->CRC16Update=0xFFFF;
	
	if(mdRTUSerialInitFun!=NULL){
		mdRTUSerialInitFun(pModbusRTU,baud, dataBits,stopBits,parity);
	}
	return ;
}
/*******************************************************
*
* Function name :MDS_RTU_SetWriteListenFun
* Description        :�ú�����������һ���ص�������������д�ӻ���ַʱ�����õĺ�����������
* Parameter         :
*        @pModbus_RTU            �ӻ��ṹ��ָ��
*        @wFun    ���õĻص�����
* Return          : ��
**********************************************************/
void MDS_RTU_SetWriteListenFun(PModbusS_RTU pModbus_RTU,MDSWriteFunciton wFun){
	if(pModbus_RTU==NULL){return ;}
	pModbus_RTU->mdsWriteFun=wFun;
}
/*******************************************************
*
* Function name :MDS_RTU_TimeHandler
* Description        :�ú�����Ҫ�ڶ�ʱ���ж��е��ã��жϼ��ʱ��100US
* Parameter         :
*        @obj            �ӻ��ṹ��ָ��
* Return          : ��
**********************************************************/
void MDS_RTU_TimeHandler(void *obj){
	uint32 tempTick=0;
	uint8 overFlag=0;
	PModbusS_RTU pModbusRTU=obj;
	if(!pModbusRTU){ return; }
	
	pModbusRTU->timesTick++;
	
	if(pModbusRTU->timesTick==0xFFFFFFFF){/*����� pModbusRTU->lastTimesTick==0xFFFFFFFF*/
		tempTick=0xFFFFFFFF-pModbusRTU->lastSendTimes;
		pModbusRTU->timesTick=tempTick; /*ϵͳʱ��ƫ��*/
		pModbusRTU->lastSendTimes=0;/*�ϴη���ʱ������*/
		overFlag=1;
	}
	
	if(pModbusRTU->lastTimesTick==0xFFFFFFFF){return ;}/*�Ѿ���ʼ���հ�*/
	if(overFlag){ /*ʱ�����*/
		pModbusRTU->timesTick += 0xFFFFFFFF-pModbusRTU->lastTimesTick;/*����ʱʱ��ƫ��*/
		pModbusRTU->lastTimesTick = tempTick; 
	}
	if((pModbusRTU->timesTick - pModbusRTU->lastTimesTick >= pModbusRTU->frameIntervalTime)){
		uint16 msgLen;
		uint16 i;
		uint8	 byte;
		if(pModbusRTU->CRC16Update!=0x0000){
			/*CRC����*/
			MDResetQueue(&(pModbusRTU->mdSqQueue));
			pModbusRTU->lastTimesTick=0xFFFFFFFF;
			return ;
		}
		/*һ֡����*/
		/*���ݴ�����е���Ϣ�ᵽ��Ϣ���������*/
		msgLen=MDQueueLength(&(pModbusRTU->mdSqQueue));
		for(i=0;i<msgLen;i++){
			/*ȡ����*/
			if(MDdeQueue(&(pModbusRTU->mdSqQueue),&byte)==FALSE){return ;}
			/*�Ž�ȥ*/
			if(MDenQueue(&(pModbusRTU->mdMsgSqQueue),byte)==FALSE){return ;}
		}
		pModbusRTU->lastTimesTick=0xFFFFFFFF;
	}
}

/*******************************************************
*
* Function name :MDS_RTU_RecvByte
* Description        :�ú����ڴ����ж��е��ã������յ�һ���ֽں���øú���
* Parameter         :
*        @obj            �ӻ��ṹ��ָ��
*        @byte    ���յ��ĵ��ֽ�
* Return          : ��
**********************************************************/
void MDS_RTU_RecvByte(void *obj,uint8 byte){
	PModbusS_RTU pModbusRTU=obj;
	if(!pModbusRTU){ return; }
	/*��������������*/
	if(MDenQueue(&(pModbusRTU->mdSqQueue),byte)==FALSE){
		return ;
	}
	if(pModbusRTU->lastTimesTick==0xFFFFFFFF){
		pModbusRTU->CRC16Update=0xFFFF;
	}
	pModbusRTU->CRC16Update=MD_CRC16Update(pModbusRTU->CRC16Update,byte);
	/*�����ϴν��յ��ַ���ʱ���*/
	pModbusRTU->lastTimesTick=pModbusRTU->timesTick;
}
/*******************************************************
*
* Function name :MDS_RTU_AddMapItem
* Description        :�ú�������ɢӳ��������һ��ӳ���¼
* Parameter         :
*        @obj            �ӻ��ṹ��ָ��
*        @byte    ���յ��ĵ��ֽ�
* Return          : ��
**********************************************************/
BOOL MDS_RTU_AddMapItem(PModbusS_RTU pModbusRTU,PMapTableItem pMapTableItem){
	if(pModbusRTU==NULL ||pMapTableItem==NULL){
			return FALSE;
	}
	return MapTableAdd(pModbusRTU->pMapTableList, pMapTableItem,MDS_REG_COIL_ITEM_NUM);
}
#if	!MDS_USE_SEND_CACHE 
/*******************************************************
*
* Function name :MDS_RTU_SendByte
* Description        :�ӻ�����һ���ֽ�
* Parameter         :
*        @pModbusRTU            �ӻ��ṹ��ָ��
*        @byte    ��Ҫ���͵��ֽ�
* Return          : TRUE success , FALSE fail
**********************************************************/
static void MDS_RTU_SendByte(PModbusS_RTU pModbusRTU,uint8 byte){
	if(!pModbusRTU){ return; }
	TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction(&byte,1);
}
#endif
/*******************************************************
*
* Function name :MDS_RTU_SerialProcess
* Description        :�ú����ڲ����ã���ȡ���յ���һ��������
* Parameter         :
*        @pModbus_RTU            �ӻ��ṹ��ָ��
* Return          : ��
**********************************************************/
static BOOL MDS_RTU_SerialProcess(PModbusS_RTU pModbus_RTU){
	uint8 byte;
	if(!pModbus_RTU){return FALSE;}
	
	/*�Ӷ��ж�ȡһ���ֽ�*/
	if(MDdeQueue(&(pModbus_RTU->mdMsgSqQueue),&byte)==FALSE){
		return FALSE;
	}
	pModbus_RTU->serialReadCache[pModbus_RTU->serialReadCount]=byte;/*save a byte*/
	
	if(pModbus_RTU->serialReadCount>=1){/*�Ѿ���ȡ������*/
		if(
			pModbus_RTU->serialReadCache[1]>=0x01&&
			pModbus_RTU->serialReadCache[1]<=0x04
		){
			/*���Ϲ��������һ����*/
			if(pModbus_RTU->serialReadCount>5){
				/*��ȡ��REG��REG_NUM*/
				if(pModbus_RTU->serialReadCount>=7){
					pModbus_RTU->serialReadCount++;
					/*���ݶ�ȡ����*/
					return TRUE;
				}
			}
		}else if(	
			pModbus_RTU->serialReadCache[1]==5||
			pModbus_RTU->serialReadCache[1]==6
		){
			/*���Ϲ��������һ����*/
			/*д����Ȧ��д���Ĵ���*/
			if(pModbus_RTU->serialReadCount>=7){
				pModbus_RTU->serialReadCount++;
				/*���ݶ�ȡ����*/
					return TRUE;
			}	
		}else if(	
			pModbus_RTU->serialReadCache[1]==15||
			pModbus_RTU->serialReadCache[1]==16
		){
			/*���Ϲ��������һ����*/
			if(pModbus_RTU->serialReadCount>6){
				if(pModbus_RTU->serialReadCount>=
					(6+pModbus_RTU->serialReadCache[6]+2)
				){
					pModbus_RTU->serialReadCount++;
					/*���ݶ�ȡ����*/
					return TRUE;
				}
			}
		}else {/*��֧�ֵĹ�����*/
			
		}
	}
	
	if(pModbus_RTU->serialReadCount<=MDS_RTU_CMD_SIZE){
		pModbus_RTU->serialReadCount++;
	}
	
	return FALSE;
}
/*******************************************************
*
* Function name :MDS_RTU_Process
* Description        :�ú���������յ��İ�����
* Parameter         :
*        @pModbus_RTU            �ӻ��ṹ��ָ��
* Return          : ��
**********************************************************/
void MDS_RTU_Process(PModbusS_RTU pModbus_RTU){
	BOOL res;
	if(!pModbus_RTU){return ;}
	
	if(pModbus_RTU->timesTick - pModbus_RTU->lastSendTimes < pModbus_RTU->frameIntervalTime){
		/*֡���ʱ�䣬3.5T����һ֡*/
		return;
	}
	
	res = MDS_RTU_SerialProcess(pModbus_RTU);/*�Ӷ����ж�ȡһ��ָ��*/
	if(!res){
		return ;
	}
	
	if(
		pModbus_RTU->serialReadCache[0]!=0x00	/*�㲥��ַ*/
	&&
		pModbus_RTU->serialReadCache[0]!=pModbus_RTU->salveAddr
	){
		/*�����ڱ��ӻ�������*/
		goto __exit;
	}

	/*������01-04���л������ƵĴ���ṹ*/
	/*01 ��ȡ��Ȧ״̬ 	ȡ��һ���߼���Ȧ�ĵ�ǰ״̬��ON/OFF)*/
	/*02 ��ȡ����״̬ 	ȡ��һ�鿪������ĵ�ǰ״̬(ON/OFF)*/
	/*03 ��ȡ���ּĴ��� ��һ���������ּĴ�����ȡ�õ�ǰ�Ķ�����ֵ*/
	/*04 ��ȡ����Ĵ��� ��һ����������Ĵ�����ȡ�õ�ǰ�Ķ�����ֵ*/
	if(MDS_RTU_FUN_CODE(pModbus_RTU)>=0x1
		&&MDS_RTU_FUN_CODE(pModbus_RTU)<=0x4
	){
		uint16 startReg=MDS_RTU_START_REG(pModbus_RTU);
		uint16 regNum=MDS_RTU_REGS_NUM(pModbus_RTU);
		MDS_RTU_ReadDataProcess(pModbus_RTU,startReg,regNum,MDS_RTU_FUN_CODE(pModbus_RTU));
	}else if(
		MDS_RTU_FUN_CODE(pModbus_RTU)==0x5|| 
		MDS_RTU_FUN_CODE(pModbus_RTU)==0x6
	){
	/*05 ǿ�õ���Ȧ ǿ��һ���߼���Ȧ��ͨ��״̬*/
	/*06 Ԥ�õ��Ĵ��� �Ѿ������ֵװ��һ�����ּĴ���*/
		uint16 startReg=MDS_RTU_START_REG(pModbus_RTU);
		uint16 val= ((pModbus_RTU->serialReadCache[4])<<8)|pModbus_RTU->serialReadCache[5];

		MDS_RTU_WriteDataProcess(pModbus_RTU,startReg,1,MDS_RTU_FUN_CODE(pModbus_RTU),&val,2);
	}else if(	
			MDS_RTU_FUN_CODE(pModbus_RTU)==15||
			MDS_RTU_FUN_CODE(pModbus_RTU)==16
	){
	/*15 ǿ�ö���Ȧ ǿ��һ�������߼���Ȧ��ͨ��*/
	/*16 Ԥ�ö�Ĵ��� �Ѿ���Ķ�����ֵװ��һ�������ı��ּĴ���*/
		uint16 startReg=MDS_RTU_START_REG(pModbus_RTU);
		uint16 regNum=MDS_RTU_REGS_NUM(pModbus_RTU);
		uint8	 bytesNum=MDS_RTU_BYTES_NUM(pModbus_RTU);
		if(MDS_RTU_FUN_CODE(pModbus_RTU)==15){
			if(((regNum>>3)+((regNum%8)?1:0))!=bytesNum){
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_COILS,ILLEGAL_DAT_VAL);
				/*�����˳��Ȳ�ƥ����󣬶���*/
				goto __exit0;
			}
		}else {//16
			if((regNum<<1)!=bytesNum){
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_REGS,ILLEGAL_DAT_VAL);
				/*�����˳��Ȳ�ƥ����󣬶���*/
				goto __exit0;
			}
		}
		
		MDS_RTU_WriteDataProcess(pModbus_RTU,startReg,regNum,MDS_RTU_FUN_CODE(pModbus_RTU),
		(uint16*)(&(pModbus_RTU->serialReadCache[7])),pModbus_RTU->serialReadCache[6]
		);
	}

__exit0:
	pModbus_RTU->lastSendTimes=pModbus_RTU->timesTick;
__exit:
	pModbus_RTU->serialReadCount = 0;

	return ;
}

/*******************************************************
*
* Function name :MDS_RTU_SendErrorCode
* Description        :�ú������ؽ����봦���еĴ�������Ϣ������
* Parameter         :
*        @pModbus_RTU            �ӻ��ṹ��ָ��
*        @anlCode            �쳣�룬�μ�[ANLCode]
*        @errCode            ������,�μ�[ErrorCode]
* Return          : ��
**********************************************************/
static void MDS_RTU_SendErrorCode(PModbusS_RTU pModbus_RTU,ANLCode anlCode,ErrorCode errCode){
	MD_RTU_SEND_MODE(pModbus_RTU);
	MDS_START_SEND(pModbus_RTU);
	MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
	MDS_SEND_BYTE(pModbus_RTU,anlCode);
	MDS_SEND_BYTE(pModbus_RTU,errCode);
	MDS_SEND_END(pModbus_RTU);	
	MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDS_RTU_ReadDataProcess
* Description        :�ú������������Ķ���������
* Parameter         :
*        @pModbus_RTU            �ӻ��ṹ��ָ��
*        @reg            ��ȡ�ĵ�ַ��ʼ
*        @regNum            ��ȡ�����ݸ���
*        @funCode            �����Ĺ�����
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDS_RTU_ReadDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,uint16 regNum,uint8 funCode){
	uint16 i=0;
	if(pModbus_RTU==NULL){return FALSE;}
	
	for(i=0;i<MDS_REG_COIL_ITEM_NUM;i++){
		if(pModbus_RTU->pMapTableList[i]==NULL){
			continue;
		}
		if(pModbus_RTU->pMapTableList[i]->modbusAddr<=reg&&
		(pModbus_RTU->pMapTableList[i]->modbusAddr+pModbus_RTU->pMapTableList[i]->modbusDataSize)>=(reg+regNum)
		)
		{
			/*ȷ����ȡ�ķ�Χ���ڴ�ķ�Χ֮��*/
			if((funCode==1&&pModbus_RTU->pMapTableList[i]->addrType==COILS_TYPE)||
				(funCode==2&&pModbus_RTU->pMapTableList[i]->addrType==INPUT_TYPE)){
				/*ȷ���Ƕ�ȡ��bit*/
				/*�õ�λƫ��*/
				uint16 offsetAddr=reg-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
				uint16 	j;
				uint16	lastIndex=0;
				uint8	 	tempByte=0;
				MD_RTU_SEND_MODE(pModbus_RTU);
				MDS_START_SEND(pModbus_RTU);
				MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);		
				MDS_SEND_BYTE(pModbus_RTU,funCode);
				MDS_SEND_BYTE(pModbus_RTU,(regNum>>3)+((regNum%8)>0?1:0));
				for(j=offsetAddr; j<offsetAddr+regNum; j++){		
					if(((j-offsetAddr)>>3)!=lastIndex){
						MDS_SEND_BYTE(pModbus_RTU,tempByte);
						tempByte=0;
						/*��ǰд���ֽ�λ�����ϴβ�ͬ��˵����Ҫ����һ���ֽ�*/
						lastIndex=(j-offsetAddr)>>3;
					}
					if(
						MD_GET_BIT(
						MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[j>>4],j%16)
					){
							MD_SET_BIT(tempByte,j%8);
					}else{
							MD_CLR_BIT(tempByte,j%8);
					}
				}
				MDS_SEND_BYTE(pModbus_RTU,tempByte);
				MDS_SEND_END(pModbus_RTU);
				MD_RTU_RECV_MODE(pModbus_RTU);
			}else if((funCode==3&&pModbus_RTU->pMapTableList[i]->addrType==HOLD_REGS_TYPE)||
				(funCode==4&&pModbus_RTU->pMapTableList[i]->addrType==INPUT_REGS_TYPE)
			){
				/*ȷ����ȡ����reg*/
					/*�õ�uint16ƫ��*/
				uint16 j=0;
				uint16 offsetAddr=reg-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]) ;
				MD_RTU_SEND_MODE(pModbus_RTU);
				MDS_START_SEND(pModbus_RTU);
				MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MDS_SEND_BYTE(pModbus_RTU,funCode);
				MDS_SEND_BYTE(pModbus_RTU,regNum<<1);
				for(j=0;j<regNum<<1;j+=2){
					uint16 temp=MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[offsetAddr+(j>>1)];
					MDS_SEND_BYTE(pModbus_RTU,(temp>>8)&0xff);
					MDS_SEND_BYTE(pModbus_RTU,(temp)&0xff);
				}
				MDS_SEND_END(pModbus_RTU);
				MD_RTU_RECV_MODE(pModbus_RTU);
			}else { 
				/*��ַ���ܶ�ȡ*/
				continue;
			}
			return TRUE;
		}
	}
	/*��ַ�쳣*/
	MDS_RTU_SendErrorCode(pModbus_RTU,(ANLCode)(0x80+funCode),ILLEGAL_DAT_ADDR);
	return FALSE;
}
/*******************************************************
*
* Function name :MDS_RTU_WriteDataProcess
* Description        :�ú�������������д��������
* Parameter         :
*        @pModbus_RTU            �ӻ��ṹ��ָ��
*        @reg            ��ȡ�ĵ�ַ��ʼ
*        @regNum            ��ȡ�����ݸ���
*        @funCode            �����Ĺ�����
*        @data            д�������
*        @byteCount            д�������ռ�����ֽ�
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDS_RTU_WriteDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,
uint16 regNum,uint8 funCode,uint16* data,uint8 byteCount){
	uint8 res=FALSE;
	switch(funCode){
		case 5:/*д����Ȧ*/
		{
			if(data[0]==0xFF00 || data[0]==0x0000){
				if(data[0]==0xFF00){
					res=MDS_RTU_WriteBit(pModbus_RTU,reg,1,COILS_TYPE);
				}else {
					res=MDS_RTU_WriteBit(pModbus_RTU,reg,0,COILS_TYPE);
				}
				if(res){
					MD_RTU_SEND_MODE(pModbus_RTU);
					MDS_START_SEND(pModbus_RTU);
					MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
					MDS_SEND_BYTE(pModbus_RTU,funCode);
					MDS_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
					MDS_SEND_BYTE(pModbus_RTU,(reg)&0xff);
					MDS_SEND_BYTE(pModbus_RTU,((*data)>>8)&0xff);
					MDS_SEND_BYTE(pModbus_RTU,(*data)&0xff);
					MDS_SEND_END(pModbus_RTU);
					MD_RTU_RECV_MODE(pModbus_RTU);
					if(pModbus_RTU->mdsWriteFun){
						pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,1,COILS_TYPE);
					}
				}else{
					MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_SIN_COIL,ILLEGAL_DAT_ADDR);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_SIN_COIL,ILLEGAL_DAT_VAL);
			}
		}
			break;
		case 15:/*д����Ȧ*/
			res=MDS_RTU_WriteBits(pModbus_RTU, reg, regNum, data,COILS_TYPE);
			if(res){
				MD_RTU_SEND_MODE(pModbus_RTU);
				MDS_START_SEND(pModbus_RTU);
				MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MDS_SEND_BYTE(pModbus_RTU,funCode);
				MDS_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,(reg)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,((regNum)>>8)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,(regNum)&0xff);
				MDS_SEND_END(pModbus_RTU);
				MD_RTU_RECV_MODE(pModbus_RTU);
				if(pModbus_RTU->mdsWriteFun){
					pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,regNum,COILS_TYPE);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_COILS,ILLEGAL_DAT_ADDR);
			}
			break;
		case 6:/*д���Ĵ���*/
			res=MDS_RTU_WriteReg(pModbus_RTU,reg,data[0],HOLD_REGS_TYPE);
			if(res){
				MD_RTU_SEND_MODE(pModbus_RTU);
				MDS_START_SEND(pModbus_RTU);
				MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MDS_SEND_BYTE(pModbus_RTU,funCode);
				MDS_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,(reg)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,((*data)>>8)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,(*data)&0xff);
				MDS_SEND_END(pModbus_RTU);
				MD_RTU_RECV_MODE(pModbus_RTU);
				if(pModbus_RTU->mdsWriteFun){
					pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,1,HOLD_REGS_TYPE);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_SIN_REG,ILLEGAL_DAT_ADDR);
			}
			break;
		case 16:/*д��Ĵ���*/
			res=MDS_RTU_WriteRegs(pModbus_RTU,reg,regNum,data,1,HOLD_REGS_TYPE);
			if(res){
				MD_RTU_SEND_MODE(pModbus_RTU);
				MDS_START_SEND(pModbus_RTU);
				MDS_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MDS_SEND_BYTE(pModbus_RTU,funCode);
				MDS_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,(reg)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,((regNum)>>8)&0xff);
				MDS_SEND_BYTE(pModbus_RTU,(regNum)&0xff);
				MDS_SEND_END(pModbus_RTU);
				MD_RTU_RECV_MODE(pModbus_RTU);
				if(pModbus_RTU->mdsWriteFun){
					pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,regNum,HOLD_REGS_TYPE);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_REGS,ILLEGAL_DAT_ADDR);
			}
			break;
	}
	if(!res){return FALSE;}
	return TRUE;
}



