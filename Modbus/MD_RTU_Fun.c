/**
* @file 		MD_RTU_Fun.c
* @brief		��
* @details	��
* @author		zspace
* @date		2020/3/23
* @version	A001
* @par History:  ��       
*/
#include "MD_RTU_Fun.h"
#include "MD_RTU_CRC16.h"
#include "MD_RTU_Serial.h"
#include "MD_RTU_User_Fun.h"
/*�������*/
void MDS_RTU_RecvByte(void *obj,uint8 byte);
void MDS_RTU_TimeHandler(void *obj,uint32 times);

static void MDS_RTU_SendErrorCode(PModbusS_RTU pModbus_RTU,ANLCode anlCode,ErrorCode errCode);
/**/
uint8 MDS_RTU_ReadDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,uint16 regNum,uint8 funCode);
uint8 MDS_RTU_WriteDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,uint16 regNum,uint8 funCode,uint16* data,uint8 byteCount);

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
	
	for(i=0;i<REG_COIL_ITEM_NUM;i++){
		pModbusRTU->pRegCoilList[i] = NULL;
	}
	
	TO_MDBase(pModbusRTU)->mdRTUTimeHandlerFunction=MDS_RTU_TimeHandler;
	TO_MDBase(pModbusRTU)->mdRTURecByteFunction=MDS_RTU_RecvByte;
	TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction=NULL;
	TO_MDBase(pModbusRTU)->mdRTURecSendConv=NULL;
	
	pModbusRTU->mdsWriteFun=NULL;
	pModbusRTU->lastTimesTick=0xFFFFFFFF;
	pModbusRTU->lastSendTimes=0;
	pModbusRTU->timesTick=0;
	
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
/*��ʱ������*/
void MDS_RTU_TimeHandler(void *obj,uint32 times){
	PModbusS_RTU pModbusRTU=obj;
	if(!pModbusRTU){ return; }
	pModbusRTU->timesTick=times;
	/*����Ҫ����*/
	if(pModbusRTU->lastTimesTick==0xFFFFFFFF){return ;}
	if(pModbusRTU->timesTick-pModbusRTU->lastTimesTick>=pModbusRTU->frameIntervalTime){
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
/*д��ص�����*/
void MDS_RTU_SetWriteListenFun(PModbusS_RTU pModbus_RTU,MDSWriteFunciton wFun){
	if(pModbus_RTU==NULL){return ;}
	pModbus_RTU->mdsWriteFun=wFun;
}
/*�ú����������ݲ��ҷ��������*/
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
	pModbusRTU->CRC16Update=crc16_update(pModbusRTU->CRC16Update,byte);
	/*�����ϴν��յ��ַ���ʱ���*/
	pModbusRTU->lastTimesTick=pModbusRTU->timesTick;
}
/*����һ���ֽ�*/
static void MDS_RTU_SendByte(PModbusS_RTU pModbusRTU,uint8 byte){
	if(!pModbusRTU){ return; }
	TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction(&byte,1);
}
/*�ڲ����õĺ���*/
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
	
	if(pModbus_RTU->serialReadCache[0]!=pModbus_RTU->salveAddr){
		/*�����ڱ��ӻ�������*/
		goto __exit;
	}

	/*������01-04���л������ƵĴ���ṹ*/
	//01 ��ȡ��Ȧ״̬ 	ȡ��һ���߼���Ȧ�ĵ�ǰ״̬��ON/OFF)
	//02 ��ȡ����״̬ 	ȡ��һ�鿪������ĵ�ǰ״̬(ON/OFF)
	//03 ��ȡ���ּĴ��� ��һ���������ּĴ�����ȡ�õ�ǰ�Ķ�����ֵ
	//04 ��ȡ����Ĵ��� ��һ����������Ĵ�����ȡ�õ�ǰ�Ķ�����ֵ
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
	//05 ǿ�õ���Ȧ ǿ��һ���߼���Ȧ��ͨ��״̬
	//06 Ԥ�õ��Ĵ��� �Ѿ������ֵװ��һ�����ּĴ���
		uint16 startReg=MDS_RTU_START_REG(pModbus_RTU);
		uint16 val= ((pModbus_RTU->serialReadCache[4])<<8)|pModbus_RTU->serialReadCache[5];

		MDS_RTU_WriteDataProcess(pModbus_RTU,startReg,1,MDS_RTU_FUN_CODE(pModbus_RTU),&val,2);
	}else if(	
			MDS_RTU_FUN_CODE(pModbus_RTU)==15||
			MDS_RTU_FUN_CODE(pModbus_RTU)==16
	){
	//15 ǿ�ö���Ȧ ǿ��һ�������߼���Ȧ��ͨ��
	//16 Ԥ�ö�Ĵ��� �Ѿ���Ķ�����ֵװ��һ�������ı��ּĴ���
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

static void MDS_RTU_SendErrorCode(PModbusS_RTU pModbus_RTU,ANLCode anlCode,ErrorCode errCode){
		MSD_START_SEND(pModbus_RTU);
		MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
		MSD_SEND_BYTE(pModbus_RTU,anlCode);
		MSD_SEND_BYTE(pModbus_RTU,errCode);
		MSD_SEND_END(pModbus_RTU);	
}
/*�����ݴ���*/
uint8 MDS_RTU_ReadDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,uint16 regNum,uint8 funCode){
	uint16 i=0;
	if(pModbus_RTU==NULL){return FALSE;}
	
	for(i=0;i<REG_COIL_ITEM_NUM;i++){
		if(pModbus_RTU->pRegCoilList[i]==NULL){
			continue;
		}
		if(pModbus_RTU->pRegCoilList[i]->modbusAddr<=reg&&
		(pModbus_RTU->pRegCoilList[i]->modbusAddr+pModbus_RTU->pRegCoilList[i]->modbusDataSize)>=(reg+regNum)
		)
		{
			/*ȷ����ȡ�ķ�Χ���ڴ�ķ�Χ֮��*/
			if((funCode==1||funCode==2) && pModbus_RTU->pRegCoilList[i]->addrType==BIT_TYPE){
				/*ȷ���Ƕ�ȡ��bit*/
				/*�õ�λƫ��*/
				uint16 offsetAddr=reg-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pRegCoilList[i]);
				uint16 	j;
				uint16	lastIndex=0;
				uint8	 	tempByte=0;
				MSD_START_SEND(pModbus_RTU);
				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);		
				MSD_SEND_BYTE(pModbus_RTU,funCode);
				MSD_SEND_BYTE(pModbus_RTU,(regNum>>3)+((regNum%8)>0?1:0));
				for(j=offsetAddr; j<offsetAddr+regNum; j++){		
					if(((j-offsetAddr)>>3)!=lastIndex){
						MSD_SEND_BYTE(pModbus_RTU,tempByte);
						tempByte=0;
						/*��ǰд���ֽ�λ�����ϴβ�ͬ��˵����Ҫ����һ���ֽ�*/
						lastIndex=(j-offsetAddr)>>3;
					}
					if(
						MD_GET_BIT(
						MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pRegCoilList[i])[j>>4],j%16)
					){
							MD_SET_BIT(tempByte,j%8);
					}else{
							MD_CLR_BIT(tempByte,j%8);
					}
				}
				MSD_SEND_BYTE(pModbus_RTU,tempByte);
				MSD_SEND_END(pModbus_RTU);
			}else if((funCode==3||funCode==4) && pModbus_RTU->pRegCoilList[i]->addrType==REG_TYPE){
				/*ȷ����ȡ����reg*/
					/*�õ�uint16ƫ��*/
				uint16 j=0;
				uint16 offsetAddr=reg-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pRegCoilList[i]) ;
				MSD_START_SEND(pModbus_RTU);
				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MSD_SEND_BYTE(pModbus_RTU,funCode);
				MSD_SEND_BYTE(pModbus_RTU,regNum<<1);
				for(j=0;j<regNum<<1;j+=2){
					uint16 temp=MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pRegCoilList[i])[offsetAddr+(j>>1)];
					MSD_SEND_BYTE(pModbus_RTU,(temp>>8)&0xff);
					MSD_SEND_BYTE(pModbus_RTU,(temp)&0xff);
				}
				MSD_SEND_END(pModbus_RTU);

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
uint8 MDS_RTU_WriteDataProcess(PModbusS_RTU pModbus_RTU,uint16 reg,
uint16 regNum,uint8 funCode,uint16* data,uint8 byteCount){
	uint8 res=FALSE;
	switch(funCode){
		case 5:/*д����Ȧ*/
		{
			if(data[0]==0xFF00 || data[0]==0x0000){
				if(data[0]==0xFF00){
					res=MDS_RTU_WriteBit(pModbus_RTU,reg,1);
				}else {
					res=MDS_RTU_WriteBit(pModbus_RTU,reg,0);
				}
				if(res){
					MSD_START_SEND(pModbus_RTU);
					MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
					MSD_SEND_BYTE(pModbus_RTU,funCode);
					MSD_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
					MSD_SEND_BYTE(pModbus_RTU,(reg)&0xff);
					MSD_SEND_BYTE(pModbus_RTU,((*data)>>8)&0xff);
					MSD_SEND_BYTE(pModbus_RTU,(*data)&0xff);
					MSD_SEND_END(pModbus_RTU);
					if(pModbus_RTU->mdsWriteFun){
						pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,1,BIT_TYPE);
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
			res=MDS_RTU_WriteBits(pModbus_RTU, reg, regNum, data);
			if(res){
				MSD_START_SEND(pModbus_RTU);
				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MSD_SEND_BYTE(pModbus_RTU,funCode);
				MSD_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,(reg)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,((regNum)>>8)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,(regNum)&0xff);
				MSD_SEND_END(pModbus_RTU);
				if(pModbus_RTU->mdsWriteFun){
					pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,regNum,BIT_TYPE);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_COILS,ILLEGAL_DAT_ADDR);
			}
			break;
		case 6:/*д���Ĵ���*/
			res=MDS_RTU_WriteReg(pModbus_RTU,reg,data[0]);
			if(res){
				MSD_START_SEND(pModbus_RTU);
				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MSD_SEND_BYTE(pModbus_RTU,funCode);
				MSD_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,(reg)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,((*data)>>8)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,(*data)&0xff);
				MSD_SEND_END(pModbus_RTU);
				if(pModbus_RTU->mdsWriteFun){
					pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,1,REG_TYPE);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_SIN_REG,ILLEGAL_DAT_ADDR);
			}
			break;
		case 16:/*д��Ĵ���*/
			res=MDS_RTU_WriteRegs(pModbus_RTU,reg,regNum,data,1);
			if(res){
				MSD_START_SEND(pModbus_RTU);
				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
				MSD_SEND_BYTE(pModbus_RTU,funCode);
				MSD_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,(reg)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,((regNum)>>8)&0xff);
				MSD_SEND_BYTE(pModbus_RTU,(regNum)&0xff);
				MSD_SEND_END(pModbus_RTU);
				if(pModbus_RTU->mdsWriteFun){
					pModbus_RTU->mdsWriteFun(pModbus_RTU,reg,regNum,REG_TYPE);
				}
			}else{
				MDS_RTU_SendErrorCode(pModbus_RTU,WRITE_REGS,ILLEGAL_DAT_ADDR);
			}
			break;
	}
	if(!res){return FALSE;}
	return TRUE;
	//	uint16 i=0;
//	if(pModbus_RTU==NULL){return FALSE;}
//	
//	for(i=0;i<REG_COIL_ITEM_NUM;i++){
//		if(pModbus_RTU->pRegCoilList[i]==NULL){
//			continue;
//		}
//		if(pModbus_RTU->pRegCoilList[i]->modbusAddr<=reg&&
//		(pModbus_RTU->pRegCoilList[i]->modbusAddr+pModbus_RTU->pRegCoilList[i]->modbusDataSize)>=(reg+regNum)
//		)
//		{
//			/*ȷ����ȡ�ķ�Χ���ڴ�ķ�Χ֮��*/
//			if((funCode==5 &&pModbus_RTU->pRegCoilList[i]->addrType==BIT_TYPE)
//			||(funCode==6 &&pModbus_RTU->pRegCoilList[i]->addrType==REG_TYPE)
//				){
//				
//				uint16 offsetAddr=reg-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pRegCoilList[i]);
//				if(funCode==5){
//					if(data[0]==0xff00){
//						MDS_SET_BIT(
//							MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pRegCoilList[i])[offsetAddr>>4],offsetAddr%16);
//					}else if(data[0]==0x0000){
//						MDS_CLR_BIT(
//							MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pRegCoilList[i])[offsetAddr>>4],offsetAddr%16);
//					}
//				}else {
//					MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pRegCoilList[i])[offsetAddr]=data[0];
//				}
//				MSD_START_SEND(pModbus_RTU);
//				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
//				MSD_SEND_BYTE(pModbus_RTU,funCode);
//				MSD_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
//				MSD_SEND_BYTE(pModbus_RTU,(reg)&0xff);
//				MSD_SEND_BYTE(pModbus_RTU,((*data)>>8)&0xff);
//				MSD_SEND_BYTE(pModbus_RTU,(*data)&0xff);
//				MSD_SEND_END(pModbus_RTU);
//			}else if(
//				(funCode==15 &&pModbus_RTU->pRegCoilList[i]->addrType==BIT_TYPE)
//			||(funCode==16 &&pModbus_RTU->pRegCoilList[i]->addrType==REG_TYPE)
//				){
//				uint16 offsetAddr=reg-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pRegCoilList[i]);
//				if(funCode==15){/*ǿ�ö���Ȧ*/
//					uint16 j;
//					for(j=0;j<regNum;j++){
//						uint8 *byteData=(uint8*)data;
//						if(
//							MDS_GET_BIT( byteData[j>>3] ,j%8)
//						){
//							MDS_SET_BIT(
//								MDS_RTU_REG_COIL_ITEM_DATA(
//								pModbus_RTU->pRegCoilList[i])[(offsetAddr+j)>>4]
//							,(j+offsetAddr)%16);
//						}else{
//							MDS_CLR_BIT(
//								MDS_RTU_REG_COIL_ITEM_DATA(
//								pModbus_RTU->pRegCoilList[i])[(offsetAddr+j)>>4]
//							,(j+offsetAddr)%16);
//						}
//					}
//				}else{/*Ԥ�ö�Ĵ���*/
//					uint16 j=0;
//					for(j=0;j<regNum;j++){
//						MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pRegCoilList[i])[offsetAddr+j]=MDS_SWAP_HL(data[j]);
//					}					
//				}
//				MSD_START_SEND(pModbus_RTU);
//				MSD_SEND_BYTE(pModbus_RTU,pModbus_RTU->salveAddr);
//				MSD_SEND_BYTE(pModbus_RTU,funCode);
//				MSD_SEND_BYTE(pModbus_RTU,(reg>>8)&0xff);
//				MSD_SEND_BYTE(pModbus_RTU,(reg)&0xff);
//				MSD_SEND_BYTE(pModbus_RTU,((regNum)>>8)&0xff);
//				MSD_SEND_BYTE(pModbus_RTU,(regNum)&0xff);
//				MSD_SEND_END(pModbus_RTU);
//			}
//		}
//	}
//	return TRUE;
}



