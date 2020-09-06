/********************************************************************************
* @File name: MDM_RTU_Fun.c
* @Author: zspace
* @Emial: 1358745329@qq.com
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU ��������ģ��
* ��Դ��ַ: https://github.com/lotoohe-space/XTinyModbus
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MDM_RTU_Fun.h"
#include "MDM_RTU_Serial.h"
#include "MD_RTU_Tool.h"
#include "MD_RTU_CRC16.h"
/*********************************����******************************************/

/*********************************��������************************************/
#if !MDM_USE_SEND_CACHE
static void MDM_RTU_SendByte(PModbus_RTU pModbus_RTU,uint8 byte);
#endif
void MDM_RTU_RecvByte(void *obj,uint8 byte);
/*********************************����******************************************/

/*******************************************************
*
* Function name :MDM_RTU_Init
* Description        :Modbus RTU ������ʼ��
* Parameter         :
*        @pModbusRTU        ��������ָ��    
*        @mdRTUSerialInitFun    ���ڳ�ʼ������
*        @baud    ������
*        @dataBits    ����λ
*        @stopBits    ֹͣλ
*        @parity    ��żУ��λ
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_Init(
	PModbus_RTU pModbusRTU,
	MD_RTU_SerialInit mdRTUSerialInitFun,
	uint32 baud,
	uint8 dataBits,
	uint8 stopBits,
	uint8 parity
){
	float T;
	uint8 i;
	if(pModbusRTU==NULL){return ERR_VOID;}
	
	MDInitQueue(&(pModbusRTU->mdSqQueue));
	for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
		pModbusRTU->pMapTableList[i] = NULL;
	}
	TO_MDBase(pModbusRTU)->mdRTUTimeHandlerFunction=MDM_RTU_TimeHandler;
	/*���ݷ��ͽ����йصĺ���*/
	TO_MDBase(pModbusRTU)->mdRTUSendBytesFunction=NULL;
	TO_MDBase(pModbusRTU)->mdRTURecByteFunction=MDM_RTU_RecvByte;
	TO_MDBase(pModbusRTU)->mdRTURecSendConv=NULL;
#if MDM_USE_SEND_CACHE
	pModbusRTU->serialSendCount=0;
#endif
	/*�ϴν��յ�ʱ��,0xFFFFFFF��ʾδ��ʼ���֡*/
	pModbusRTU->lastTimesTick=0xFFFFFFFF;
	/*��ǰ��ʵʱʱ�䵥λ100US*/
	pModbusRTU->timesTick=0;
	
	T=(1.0/(float)baud)*100000;
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
	
	pModbusRTU->recvFlag=0;/*���ձ�־*/
	if(mdRTUSerialInitFun!=NULL){
		mdRTUSerialInitFun(pModbusRTU,baud, dataBits,stopBits,parity);
	}
	
	return ERR_NONE;
}
/*******************************************************
*
* Function name :MDM_RTU_CB_Init
* Description        :Modbus RTU �������Ϳ��ƿ��ʼ�������ƿ���Ҫ�����ӷ�ʱ�䣬��������Ϣά��
* Parameter         :
*        @pModbusRTUCB        ���Ϳ��ƿ����ָ��    
*        @pModbusRTU    ��������ָ��
*        @sendIntervalTime    ���ͼ��ʱ��
*        @sendOverTime    ���ͳ�ʱʱ��
*        @RTTimes    �ش�����
* Return          : ��
**********************************************************/
void MDM_RTU_CB_Init(
	PModbus_RTU_CB 	pModbusRTUCB
	,PModbus_RTU 		pModbusRTU
	,uint32 				sendIntervalTime
	,uint32					sendOverTime
	,uint8 					RTTimes
){
	if(pModbusRTUCB==NULL){return ;}
	pModbusRTUCB->sendIntervalTime=sendIntervalTime;
	pModbusRTUCB->pModbus_RTU=pModbusRTU;
	pModbusRTUCB->sendTimeTick=0;
	pModbusRTUCB->sendOverTime=sendOverTime;
	pModbusRTUCB->RTTimes=RTTimes;
	pModbusRTUCB->sendFlag=0;
	pModbusRTU->parentObj=NULL;
}
/*******************************************************
*
* Function name :MDM_RTU_CB_OverTimeReset
* Description        :���Ϳ��ƿ鳬ʱ��λ
* Parameter         :
*        @pModbusRTUCB        ���Ϳ��ƿ����ָ��    
* Return          : ��
**********************************************************/
void MDM_RTU_CB_OverTimeReset(PModbus_RTU_CB 	pModbusRTUCB){
	if(pModbusRTUCB==NULL){return ;}
	pModbusRTUCB->RTCount=0;
	pModbusRTUCB->sendFlag=0;
}
/*******************************************************
*
* Function name :MDM_RTU_TimeHandler
* Description        :��ʱ����������ʱ��λ100US
* Parameter         :
*        @obj        ��������ָ��    
* Return          : ��
**********************************************************/
void MDM_RTU_TimeHandler(void *obj){
	PModbus_RTU pModbusRTU=obj;
	if(!pModbusRTU){ return; }
	pModbusRTU->timesTick++;
	/*����Ҫ����*/
	if(pModbusRTU->lastTimesTick==0xFFFFFFFF){return ;}
	if(pModbusRTU->timesTick-pModbusRTU->lastTimesTick>=pModbusRTU->frameIntervalTime){
		if(pModbusRTU->CRC16Update!=0x0000){
			/*CRC����*/
			MDResetQueue(&(pModbusRTU->mdSqQueue));
			pModbusRTU->lastTimesTick=0xFFFFFFFF;
			return ;
		}
		/*һ֡����*/
		pModbusRTU->recvFlag=1;
		pModbusRTU->lastTimesTick=0xFFFFFFFF;
	}
}
/*******************************************************
*
* Function name :MDM_RTU_RecvByte
* Description        :�ú����������ݲ��ҷ��������
* Parameter         :
*        @obj        ��������ָ��    
*        @byte       ���յ�һ���ֽ�    
* Return          : ��
**********************************************************/
void MDM_RTU_RecvByte(void *obj,uint8 byte){
	PModbus_RTU pModbusRTU=obj;
	if(!pModbusRTU){ return; }
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
#if !MDM_USE_SEND_CACHE
/*******************************************************
*
* Function name :MDM_RTU_SendByte
* Description        :����һ���ֽ�
* Parameter         :
*        @pModbus_RTU        ��������ָ��    
*        @byte       ���͵�һ���ֽ�    
* Return          : ��
**********************************************************/
static void MDM_RTU_SendByte(PModbus_RTU pModbus_RTU,uint8 byte){
	if(!pModbus_RTU){ return; }
	TO_MDBase(pModbus_RTU)->mdRTUSendBytesFunction(&byte,1);
}
#endif
/*******************************************************
*
* Function name :MDM_RTU_AddMapItem
* Description        :�ú�������ɢӳ��������һ��ӳ���¼
* Parameter         :
*        @pModbusRTU      �����ṹ��ָ��
*        @pRegCoilItem    ��Ҫ��ӵ���
* Return          : ��
**********************************************************/
BOOL MDM_RTU_AddMapItem(PModbus_RTU pModbusRTU,PMapTableItem pMapTableItem){
	if(pModbusRTU==NULL || pMapTableItem==NULL){
			return FALSE;
	}
	return MapTableAdd(pModbusRTU->pMapTableList, pMapTableItem,MDM_REG_COIL_ITEM_NUM);
}

/*******************************************************
*
* Function name :MDM_RTU_ReadByte
* Description        :�ӽ��ն����л�ȡ����
* Parameter         :
*        @pModbus_RTU        ��������ָ��    
*        @res       ��ȡ���ݻ���
*        @len       ��ȡ�����ݳ���    
* Return          : ��
**********************************************************/
MDError MDM_RTU_ReadByte(PModbus_RTU pModbusRTU,uint8 *res,uint8 len){
	uint8 i;
	if(res==NULL){return ERR_VOID;}
	for(i=0;i<len;i++){
		if(!MDdeQueue(&(pModbusRTU->mdSqQueue),(res+i))){
			return ERR_QUEUE;
		}
	}
	return ERR_NONE;
}
/*******************************************************
*
* Function name :MDM_RTU_ReadUint16
* Description        :�ӽ��ն����л�ȡ����
* Parameter         :
*        @pModbus_RTU        ��������ָ��    
*        @res       ��ȡ���ݻ���
*        @len       ��ȡ�����ݳ���    
* Return          : ��
**********************************************************/
MDError MDM_RTU_ReadUint16(PModbus_RTU pModbusRTU,uint16 *res,uint8 len){
	uint8 i;
	uint8 byte;
	if(res==NULL){return ERR_VOID;}
	for(i=0;i<len;i++){
		if(!MDdeQueue(&(pModbusRTU->mdSqQueue),&byte)){
			return ERR_QUEUE;
		}
		res[i]=byte<<8;
		if(!MDdeQueue(&(pModbusRTU->mdSqQueue),&byte)){
			return ERR_QUEUE;
		}
		res[i]|=byte;
	}
	return ERR_NONE;
}
/*******************************************************
*
* Function name :MDM_RTU_ReadFun
* Description        :�ú������Ͷ��������ָ��
* Parameter         :
*        @pModbus_RTU        ��������ָ��    
*        @funCode       ������
*        @slaveAddr       �ӻ���ַ    
*        @startAddr       ��ȡ��ʼ��ַ    
*        @numOf       ��ȡ�ĸ���    
* Return          : ��
**********************************************************/
static void MDM_RTU_ReadFun(PModbus_RTU pModbus_RTU,uint8 funCode,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	MD_RTU_SEND_MODE(pModbus_RTU);
	MEM_RTU_START_EN(pModbus_RTU);
	MEM_RTU_EN_QUEUE(pModbus_RTU,slaveAddr);
	MEM_RTU_EN_QUEUE(pModbus_RTU,funCode);
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(startAddr));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(startAddr));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(numOf));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(numOf));
	MEM_RTU_END_EN(pModbus_RTU);
	MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleFun
* Description        :д������Ȧ��Ĵ�������
* Parameter         :
*        @pModbus_RTU        ��������ָ��    
*        @funCode       ������
*        @slaveAddr       �ӻ���ַ    
*        @startAddr       д��ʼ��ַ    
*        @value       д���ֵ  
* Return          : ��
**********************************************************/
static void MDM_RTU_WriteSingleFun(PModbus_RTU pModbus_RTU,uint8 funCode,uint8 slaveAddr,uint16 startAddr,uint16 value){
	MD_RTU_SEND_MODE(pModbus_RTU);
	MEM_RTU_START_EN(pModbus_RTU);
	MEM_RTU_EN_QUEUE(pModbus_RTU,slaveAddr);
	MEM_RTU_EN_QUEUE(pModbus_RTU,funCode);
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(startAddr));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(startAddr));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(value));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(value));
	MEM_RTU_END_EN(pModbus_RTU);
	MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDM_RTU_WriteFun
* Description        :д�����Ȧ�����Ĵ���
* Parameter         :
*        @pModbus_RTU        ��������ָ��    
*        @funCode       ������
*        @slaveAddr       �ӻ���ַ    
*        @startAddr       д��ʼ��ַ    
*        @numOf       д������ݸ���
*        @data       д������� 
* Return          : ��
**********************************************************/
static void MDM_RTU_WriteFun(PModbus_RTU pModbus_RTU,
	uint8 funCode,uint8 slaveAddr,uint16 startAddr,uint8 numOf,uint8 *data){
	uint16 i;
	uint8 wLen;
	MD_RTU_SEND_MODE(pModbus_RTU);
	MEM_RTU_START_EN(pModbus_RTU);
	MEM_RTU_EN_QUEUE(pModbus_RTU,slaveAddr);
	MEM_RTU_EN_QUEUE(pModbus_RTU,funCode);
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(startAddr));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(startAddr));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_H_BYTE(numOf));
	MEM_RTU_EN_QUEUE(pModbus_RTU,MD_L_BYTE(numOf));
	if(funCode==15){
		wLen=(numOf>>3) + ((numOf%8)?1:0);
	}else if(funCode==16){
		wLen=numOf<<1;
	}
	MEM_RTU_EN_QUEUE(pModbus_RTU,wLen);
	if(funCode == 15){
		for(i=0;i<wLen;i++){
			MEM_RTU_EN_QUEUE(pModbus_RTU,data[i]);
		}
	}else if(funCode == 16){
		uint16 *tempData= (uint16*)data;
		for(i=0;i<numOf;i++){
			MEM_RTU_EN_QUEUE(pModbus_RTU,(tempData[i]>>8));
			MEM_RTU_EN_QUEUE(pModbus_RTU,(tempData[i]&0xff));
		}
	} 
	MEM_RTU_END_EN(pModbus_RTU);
	MD_RTU_RECV_MODE(pModbus_RTU);
}
/*******************************************************
*
* Function name :MDM_RTU_InsideWriteBits
* Description        :����ɢӳ����д��bit
* Parameter         :
*        @obj        ��������ָ��    
*        @modbusAddr   ��ɢӳ���modbus��ַ
*        @numOf       д�����    
*        @bit       д�������    
*        @opAddrType       д��ĵ�ַ����(COILS_TYPE,INPUT_TYPE)���μ�[AddrType]
* Return          : TRUE , FALSE
**********************************************************/
BOOL MDM_RTU_InsideWriteBits(
	void* obj,
	uint16 modbusAddr,
	uint16 numOf, 
	uint8 *bit, 
	AddrType opAddrType,
	uint8 devAddr
){
	uint16 i;
	PModbus_RTU pModbus_RTU = obj;
	if(pModbus_RTU==NULL){return FALSE;}
	if(opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE){return FALSE;}
	
	for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
		if(pModbus_RTU->pMapTableList[i]==NULL){
			continue;
		}
		/*����豸��*/
		if(devAddr!=pModbus_RTU->pMapTableList[i]->devAddr){continue;}
		
		if(pModbus_RTU->pMapTableList[i]->modbusAddr<=modbusAddr&&
		(pModbus_RTU->pMapTableList[i]->modbusAddr+pModbus_RTU->pMapTableList[i]->modbusDataSize)>=(modbusAddr+numOf)
		){
			if(pModbus_RTU->pMapTableList[i]->addrType==opAddrType){/*������BIT����*/
				uint16 offsetAddr=modbusAddr-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
				uint16 j;
				for(j=0;j<numOf;j++){
					if(
						MD_GET_BIT( bit[j>>3] ,j%8)
					){
						MD_SET_BIT(
							MDS_RTU_REG_COIL_ITEM_DATA(
							pModbus_RTU->pMapTableList[i])[(offsetAddr+j)>>4]
						,(j+offsetAddr)%16);
					}else{
						MD_CLR_BIT(
							MDS_RTU_REG_COIL_ITEM_DATA(
							pModbus_RTU->pMapTableList[i])[(offsetAddr+j)>>4]
						,(j+offsetAddr)%16);
					}
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}
/*******************************************************
*
* Function name :MDM_RTU_InsideWriteRegs
* Description        :д��ɢ�Ĵ���
* Parameter         :
*        @obj        ��������ָ��    
*        @modbusAddr   ��ɢӳ���modbus��ַ
*        @numOf       д�����    
*        @reg       д�������    
*        @isBigE       ��˻���С�˷�ʽ  
*        @opAddrType       д��ĵ�ַ����(HOLD_REGS_TYPE,INPUT_REGS_TYPE)���μ�[AddrType]
* Return          : TRUE , FALSE
**********************************************************/
BOOL MDM_RTU_InsideWriteRegs(
void* obj,
uint16 modbusAddr,
uint16 numOf, 
uint16 *reg,
uint8 isBigE, 
AddrType opAddrType,
uint8 devAddr
){
	uint16 i;
	PModbus_RTU pModbus_RTU = obj;
	if(pModbus_RTU==NULL){return FALSE;}
	if(opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE){return FALSE;}
	
	for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
		if(pModbus_RTU->pMapTableList[i]==NULL){
			continue;
		}
		/*����豸��*/
		if(devAddr!=pModbus_RTU->pMapTableList[i]->devAddr){continue;}
		
		if(pModbus_RTU->pMapTableList[i]->modbusAddr<=modbusAddr&&
		(pModbus_RTU->pMapTableList[i]->modbusAddr+pModbus_RTU->pMapTableList[i]->modbusDataSize)>=(modbusAddr+numOf)
		){
			if(pModbus_RTU->pMapTableList[i]->addrType==opAddrType){/*������REG����*/
				uint16 offsetAddr=modbusAddr-MDS_RTU_REG_COIL_ITEM_ADDR(pModbus_RTU->pMapTableList[i]);
				uint16 j=0;
				for(j=0;j<numOf;j++){
					MDS_RTU_REG_COIL_ITEM_DATA(pModbus_RTU->pMapTableList[i])[offsetAddr+j]=
					isBigE?MD_SWAP_HL(reg[j]):reg[j];
				}		
				return TRUE;
			}
		}
	}
	return FALSE;
}
/*******************************************************
*
* Function name :MDM_RTU_NB_RW
* Description        :������ʽ��д
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @funCode   �����룬�μ�[ModbusFunCode]
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ��д��ʼ��ַ    
*        @numOf       ��д���ݸ���  
*        @wData       �����д�빦���룬��������д�������
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_RW(
	PModbus_RTU_CB pModbus_RTU_CB,
	ModbusFunCode funCode,
	uint8 slaveAddr,
	uint16 startAddr,
	uint16 numOf,
	void *wData
){
	MDError errRes;
	uint8 index;
	uint16 wAddr;
	errRes = ERR_NONE;
	if(pModbus_RTU_CB==NULL){return ERR_VOID;}
	if(pModbus_RTU_CB->pModbus_RTU==NULL){return ERR_VOID;}
	
	if(	pModbus_RTU_CB->pModbus_RTU->parentObj!=NULL &&
			pModbus_RTU_CB!=pModbus_RTU_CB->pModbus_RTU->parentObj){
			return ERR_IDLE;
	}
	
	if(pModbus_RTU_CB->sendFlag==0){/*��û���͹��������Ѿ����ͳɹ�*/
		/*������ն���*/
		MDResetQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue));
		if(funCode>=1 && funCode<=4){
			/*��û�з��͹�������*/
			MDM_RTU_ReadFun(pModbus_RTU_CB->pModbus_RTU,funCode,slaveAddr,startAddr,numOf);
		}
		else if(funCode==5||funCode==6){
			if(numOf>=1){/*���ȱ�����ڵ���1*/
				MDM_RTU_WriteSingleFun(pModbus_RTU_CB->pModbus_RTU,funCode,slaveAddr,startAddr,((uint16*)(wData))[0]);
			}
		}else if(funCode==15||funCode==16){
			MDM_RTU_WriteFun(pModbus_RTU_CB->pModbus_RTU,funCode,slaveAddr,startAddr,numOf,(uint8*)(wData));
		}
		/*���ó�ʱʱ�����*/
		pModbus_RTU_CB->sendTimeTick=pModbus_RTU_CB->pModbus_RTU->timesTick;
		
		/*�Ѿ��������ݣ��ȴ���ʱ���*/
		//MD_SET_SENDED_FLAG(pModbus_RTU_CB);
		pModbus_RTU_CB->sendFlag=1;
		/*���øÿ����ڹ���*/
		pModbus_RTU_CB->pModbus_RTU->parentObj=pModbus_RTU_CB;
		return ERR_SEND_FIN;
	}else if(pModbus_RTU_CB->RTCount<pModbus_RTU_CB->RTTimes){/*�Ѿ����͹�������û�г��ַ��ʹ�������*/
		
		if(pModbus_RTU_CB->pModbus_RTU->recvFlag){/*�յ�������*/
			uint8 byte;
			uint8 funCodeByte=0;
			/*�����־*/
			pModbus_RTU_CB->pModbus_RTU->recvFlag=0;
			
			if(!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&byte)){
				errRes =  ERR_DATA_LEN;
				goto _exit;
			}
			/*����յ����ݣ�������ݽ��д���*/
			if(slaveAddr!=byte){
				errRes =  ERR_SLAVE_ADDR;
				goto _exit;
			}
			if(!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&funCodeByte)){
				errRes =  ERR_DATA_LEN;
				goto _exit;
			}
			
			switch(funCodeByte){
				case 0x1:/*����Ȧ�ɹ�*/
				case 0x2:/*��������ɢ��*/
				{
					uint16 i;
					MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&byte);
					if((byte+2)!=MDQueueLength(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue))){
						/*���ȴ���*/
						errRes =  ERR_DATA_LEN;
						goto _exit;
					}
					index = numOf;
					wAddr=startAddr;
					for(i=0;i<byte;i++){
						uint8 rByte;
						if(!MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&rByte)){
								/*���ȴ���*/
								errRes =  ERR_DATA_LEN;
								goto _exit;
						}
						/*���δ�С�ڵ���8bit*/
						if(!MDM_RTU_InsideWriteBits(pModbus_RTU_CB->pModbus_RTU,wAddr,((index<8)?index:8), &rByte,(AddrType)funCodeByte,slaveAddr)){
							errRes= ERR_DATA_SAVE; 
							goto _exit;
						}
						wAddr += ((index<8)?index:8);
						index-=8;
					}
//					/*�ͷ�ͨ��*/
//					pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
					/*�ӻ��յ�*/
						pModbus_RTU_CB->sendFlag=2;
					errRes= ERR_RW_FIN;
					goto _exit;
				}
				case 0x3:/*�����ּĴ���*/
				case 0x4:/*������Ĵ���*/
				{
					uint16 i;
					uint16 len;
					MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&byte);
					if((byte+2)!=MDQueueLength(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue))){
						/*���ȴ���*/
						errRes= ERR_DATA_LEN;
						goto _exit;
					}
					len=byte>>1;
					for(i=0;i<len ;i++){
						uint16 	wTemp;
						uint8		rByte;
						MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&rByte);
						wTemp=(rByte<<8);
						MDdeQueue(&(pModbus_RTU_CB->pModbus_RTU->mdSqQueue),&rByte);
						wTemp|=rByte;
						if(!MDM_RTU_InsideWriteRegs(pModbus_RTU_CB->pModbus_RTU,startAddr+i,1,&wTemp,0,(AddrType)funCodeByte,slaveAddr)){
							errRes= ERR_DATA_SAVE;
							goto _exit;
						}
					}
						pModbus_RTU_CB->sendFlag=2;
					errRes= ERR_RW_FIN;
					goto _exit;
				}	
				
				case 0x5:/*д����Ȧ*/
				case 0x6:	/*д�����Ĵ���*/
				{
					uint16 res;
					if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
						errRes= ERR_DATA_LEN;
						goto _exit;
					}
					/*���ص�ַ��ƥ�����*/
					if(res!=startAddr){
						errRes= ERR_WRITE_COIL;
						goto _exit;
					}
					if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
						errRes= ERR_DATA_LEN;
						goto _exit;
					}
					/*�������ݲ�ƥ��*/
					if(res!=*((uint16*)wData)){
						errRes= ERR_WRITE_COIL;
						goto _exit;
					}
						pModbus_RTU_CB->sendFlag=2;
					errRes= ERR_RW_FIN;
					goto _exit;
				}
				case 0x0F:	/*д����Ȧ*/
				case 0x10:{	/*д��Ĵ���*/
					uint16 res;
					if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
						errRes= ERR_DATA_LEN;
						goto _exit;
					}
					/*���ص�ַ��ƥ�����*/
					if(res!=startAddr){
						errRes= ERR_WRITE_COIL; 
						goto _exit;
					}
					if(MDM_RTU_ReadUint16(pModbus_RTU_CB->pModbus_RTU,&res,1)!=ERR_NONE){
						errRes= ERR_DATA_LEN;
						goto _exit;
					}
					/*�������ݲ�ƥ��*/
					if(res!=numOf){
						errRes= ERR_WRITE_COIL;
						goto _exit;
					}
					pModbus_RTU_CB->sendFlag=2;
					errRes= ERR_RW_FIN;
					goto _exit;
				}
				case 0x81: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_COIL; goto _exit;			/*����Ȧ�쳣*/
				case 0x82: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_INPUT; goto _exit;			/*��������ɢ���쳣*/
				case 0x83: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_HOLD_REG; goto _exit;	/*�����ּĴ�������*/
				case 0x84: pModbus_RTU_CB->sendFlag=3; errRes= ERR_READ_INPUT_REG; goto _exit;	/*������Ĵ�������*/
				case 0x85: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_COIL; goto _exit;			/*д����Ȧ����*/
				case 0x86: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_REG; goto _exit;			/*д���Ĵ���*/
				case 0x8F: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_COIL; goto _exit;			/*д����Ȧ����*/	
				case 0x90: pModbus_RTU_CB->sendFlag=3; errRes= ERR_WRITE_REG; goto _exit;			/*д��Ĵ�������*/	
			}	

		}
		if(pModbus_RTU_CB->sendFlag==2){/*������ͳɹ�����д��·���*/
			/*��ʱ�ط�*/
			if(pModbus_RTU_CB->pModbus_RTU->timesTick-pModbus_RTU_CB->sendTimeTick
				>=pModbus_RTU_CB->sendIntervalTime){
				/*���·���*/
				//MD_CLR_SENDED_FLAG(pModbus_RTU_CB);
				pModbus_RTU_CB->sendFlag=0;
	//			/*�ͷ�ͨ��*/
	//			pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
				errRes= ERR_NONE;
				goto _exit;
			}
		}else if(pModbus_RTU_CB->sendFlag==1 || pModbus_RTU_CB->sendFlag==3){/*�Ѿ������ˣ����ǻ�û�յ���������ִ�г�ʱ���*/		
			/*���ճ�ʱ���*/
			if(pModbus_RTU_CB->pModbus_RTU->timesTick-pModbus_RTU_CB->sendTimeTick
				>=pModbus_RTU_CB->sendOverTime){
					/*���ó�ʱʱ�����*/
					pModbus_RTU_CB->sendTimeTick=pModbus_RTU_CB->pModbus_RTU->timesTick;
					/*���·���*/
					pModbus_RTU_CB->sendFlag=0;
					/*�ش�����+1*/
					pModbus_RTU_CB->RTCount++;
					
					/*�ͷ�ͨ��*/
					pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
					
					if(pModbus_RTU_CB->RTCount>=pModbus_RTU_CB->RTTimes){
						/*�ش���������*/
						errRes= ERR_RW_OV_TIME_ERR;
						goto _exit;
					}
					/*��ʱ��*/
					errRes= ERR_OVER_TIME;
					goto _exit;
			}
		}
	}else {
		/*�ش���������*/
		errRes= ERR_RW_OV_TIME_ERR;
		goto _exit;
	}

	if(pModbus_RTU_CB->sendFlag==1){/*�Ѿ����͹��򷵻ط�����ɱ�־*/
		return ERR_SEND_FIN; 
	}
_exit:
//	MD_CLR_SENDED_FLAG(pModbus_RTU_CB);
//	if(errRes == ERR_RW_FIN){
//		pModbus_RTU_CB->RTCount=0;
//	}
	/*�ͷ�ͨ��*/
	pModbus_RTU_CB->pModbus_RTU->parentObj=NULL;
	return errRes;
}
/*******************************************************
*
* Function name :MDM_RTU_RW
* Description        :����ʽ��д
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @funCode   �����룬�μ�[ModbusFunCode]
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ��д��ʼ��ַ    
*        @numOf       ��д���ݸ���  
*        @wData       �����д�빦���룬��������д�������
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_RW(
	PModbus_RTU_CB pModbus_RTU_CB,
	ModbusFunCode funCode,
	uint8 slaveAddr,
	uint16 startAddr,
	uint16 numOf,
	void *wData
){
	MDError res;
	do{
		res = MDM_RTU_NB_RW(pModbus_RTU_CB,funCode,slaveAddr,startAddr,numOf,wData);
		if(res != ERR_RW_FIN){/*���ִ���*/
			if(res == ERR_RW_OV_TIME_ERR){/*�ش���ʱ��*/
				/*ʹ���ش�*/
				MDM_RTU_CB_OverTimeReset(pModbus_RTU_CB);
				return res;
			}
		}
	}while(res!=ERR_RW_FIN);
	//pModbus_RTU_CB->sendFlag=0;
	return res;
}
/*******************************************************
*
* Function name :MDM_RTU_ReadCoil
* Description        :����Ȧ
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_RW(pModbus_RTU_CB,READ_COIL,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadInput
* Description        :������
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_ReadInput(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_RW(pModbus_RTU_CB,READ_INPUT,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadHoldReg
* Description        :�����ּĴ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_RW(pModbus_RTU_CB,READ_HOLD_REG,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadInputReg
* Description        :������Ĵ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_RW(pModbus_RTU_CB,READ_INPUT_REG,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleCoil
* Description        :д������Ȧ
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_WriteSingleCoil(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,BOOL boolVal){
	uint16 temp;
	temp=boolVal?0xFF00:0x0000;
	return MDM_RTU_RW(pModbus_RTU_CB,WRITE_SIN_COIL,slaveAddr,startAddr,1,(void*)(&temp));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleReg
* Description        :д�����Ĵ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_WriteSingleReg(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 val){
	return MDM_RTU_RW(pModbus_RTU_CB,WRITE_SIN_REG,slaveAddr,startAddr,1,(void*)(&val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteCoils
* Description        :д��Ȧ
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       д���ݸ���  
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_WriteCoils(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf,uint8* val){
	return MDM_RTU_RW(pModbus_RTU_CB,WRITE_COILS,slaveAddr,startAddr,numOf,(void*)(val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteRegs
* Description        :д�Ĵ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       д���ݸ���  
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_WriteRegs(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf,uint16* val){
	return MDM_RTU_RW(pModbus_RTU_CB,WRITE_REGS,slaveAddr,startAddr,numOf,(void*)(val));
};

/*******************************************************
*
* Function name :MDM_RTU_ReadCoil
* Description        :����������Ȧ
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadCoil(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_COIL,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadInput
* Description        :������������
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadInput(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_INPUT,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadHoldReg
* Description        :�����������ּĴ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadHoldReg(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_HOLD_REG,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_ReadHoldReg
* Description        :������������Ĵ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       �����ݸ���  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_ReadInputReg(PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,READ_INPUT_REG,slaveAddr,startAddr,numOf,NULL);
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleCoil
* Description        :������д������Ȧ
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteSingleCoil(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,BOOL boolVal){
	uint16 temp;
	temp=boolVal?0xFF00:0x0000;
	return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_SIN_COIL,slaveAddr,startAddr,1,(void*)(&temp));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteSingleReg
* Description        :������д�����Ĵ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteSingleReg(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 val){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_SIN_REG,slaveAddr,startAddr,1,(void*)(&val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteCoils
* Description        :������д��Ȧ
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       д���ݸ���  
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteCoils(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf,uint8* val){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_COILS,slaveAddr,startAddr,numOf,(void*)(val));
};
/*******************************************************
*
* Function name :MDM_RTU_WriteRegs
* Description        :������д�Ĵ���
* Parameter         :
*        @pModbus_RTU_CB    д����ƿ����ָ��  
*        @slaveAddr      	�ӻ���ַ    
*        @startAddr       ����ʼ��ַ    
*        @numOf       д���ݸ���  
*        @boolVal      TRUE , FALSE  
* Return          : �μ�[MDError]
**********************************************************/
MDError MDM_RTU_NB_WriteRegs(
	PModbus_RTU_CB pModbus_RTU_CB,uint8 slaveAddr,uint16 startAddr,uint16 numOf,uint16* val){
	return MDM_RTU_NB_RW(pModbus_RTU_CB,WRITE_REGS,slaveAddr,startAddr,numOf,(void*)(val));
};
	
