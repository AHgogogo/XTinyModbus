/********************************************************************************
* @File name: MD_RTU_User_Fun.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU �����û����ú���
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MDM_RTU_User_Fun.h"
#include "MDM_RTU_Fun.h"
/*********************************����******************************************/

/*******************************************************
*
* Function name :MDM_RTU_ReadBits
* Description        :��ȡ��ɢӳ���bits,���Զ�ȡһ����Ҳ���Զ�ȡ���
* Parameter         :
*        @modbusAddr        modbus�ĵ�ַ    
*        @numOf    ��Ҫ��ȡ�ĸ���
*        @opAddrType    ��ַ����(COILS_TYPE,INPUT_TYPE)���μ�[AddrType]
* Return          : 
*        @res    ���ص�ֵ
*				 �������� TRUE success , FALSE fail
**********************************************************/
BOOL MDM_RTU_ReadBits(void* obj,uint16 modbusAddr,uint16 numOf, uint8 *res, AddrType opAddrType){
	uint16 i;
	PModbus_RTU pModbusS_RTU = obj;
	if(pModbusS_RTU==NULL){return FALSE;}
		if(opAddrType != COILS_TYPE && opAddrType != INPUT_TYPE){return FALSE;}
	
	for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
		if(pModbusS_RTU->pRegCoilList[i]==NULL){
			continue;
		}
		if(pModbusS_RTU->pRegCoilList[i]->modbusAddr<=modbusAddr&&
		(pModbusS_RTU->pRegCoilList[i]->modbusAddr+pModbusS_RTU->pRegCoilList[i]->modbusDataSize)>=(modbusAddr+numOf)
		){
			if(pModbusS_RTU->pRegCoilList[i]->addrType==opAddrType){/*������BIT����*/
				uint16 	j;
				uint16 offsetAddr=modbusAddr-MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pRegCoilList[i]);
				for(j=offsetAddr; j<offsetAddr+numOf; j++){		
					if(
						MD_GET_BIT(
						MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pRegCoilList[i])[j>>4],j%16)
					){
							MD_SET_BIT(res[j>>3],j%8);
					}else{
							MD_CLR_BIT(res[j>>3],j%8);
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
* Function name :MDM_RTU_ReadRegs
* Description        :��ȡ��ɢӳ��ļĴ���,���Զ�ȡһ����Ҳ���Զ�ȡ���
* Parameter         :
*        @modbusAddr        modbus�ĵ�ַ    
*        @numOf    ��Ҫ��ȡ�ĸ���
*        @opAddrType    ��ַ����(COILS_TYPE,INPUT_TYPE)���μ�[AddrType]
* Return          : 
*        @res    ���ص�ֵ
*				 �������� TRUE success , FALSE fail
**********************************************************/
BOOL MDM_RTU_ReadRegs(void* obj,uint16 modbusAddr,uint16 numOf, uint16 *res, AddrType opAddrType){
	uint16 i;
	PModbus_RTU pModbusS_RTU = obj;
	if(pModbusS_RTU==NULL){return FALSE;}
	if(opAddrType != HOLD_REGS_TYPE && opAddrType != INPUT_REGS_TYPE){return FALSE;}
	
	for(i=0;i<MDM_REG_COIL_ITEM_NUM;i++){
		if(pModbusS_RTU->pRegCoilList[i]==NULL){
			continue;
		}
		if(pModbusS_RTU->pRegCoilList[i]->modbusAddr<=modbusAddr&&
		(pModbusS_RTU->pRegCoilList[i]->modbusAddr+pModbusS_RTU->pRegCoilList[i]->modbusDataSize)>=(modbusAddr+numOf)
		){
			if(pModbusS_RTU->pRegCoilList[i]->addrType==opAddrType){/*������BIT����*/
				uint16 	j;
				uint16 offsetAddr=modbusAddr-MDS_RTU_REG_COIL_ITEM_ADDR(pModbusS_RTU->pRegCoilList[i]);
				for(j=0;j<numOf;j++){
					res[j]=MDS_RTU_REG_COIL_ITEM_DATA(pModbusS_RTU->pRegCoilList[i])[offsetAddr+j];
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}