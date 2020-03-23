/**
* @file 		MD_RTU_APP.c
* @brief		��
* @details	��
* @author		zspace
* @date		2020/3/23
* @version	A001
* @par History:  ��       
*/
#include "MD_RTU_APP.h"
#include "MD_RTU_RegCoil.h"
#include "MD_RTU_Fun.h"
#include "MD_RTU_User_Fun.h"
#define SALVE_ADDR	0x01

uint16 regCoilData0[32]={1,2,3,4,5,6,7,8,9,10,11,12};
RegCoilItem regCoilItem0={
	.modbusAddr=0x0000,				/*MODBUS�еĵ�ַ*/
	.modbusData=regCoilData0,	/*ӳ����ڴ浥Ԫ*/
	.modbusDataSize=32,				/*ӳ��Ĵ�С*/
	.addrType=REG_TYPE				/*ӳ�������*/
};
uint16 regCoilData1[4]={0};
RegCoilItem regCoilItem1={
	.modbusAddr=0x0000,				/*MODBUS�еĵ�ַ*/
	.modbusData=regCoilData1,	/*ӳ����ڴ浥Ԫ*/
	.modbusDataSize=64,				/*ӳ��Ĵ�С*/
	.addrType=BIT_TYPE				/*ӳ�������*/
};


ModbusS_RTU modbusS_RTU={0};

BOOL MDS_RTU_APPInit(void){
	
	MDS_RTU_Init(&modbusS_RTU,SALVE_ADDR,9600,8,1,0);
	
	/*���һ����ַӳ��*/
	if(RegCoilListAdd(&modbusS_RTU, &regCoilItem0)==FALSE){
		return FALSE;
	}
	if(RegCoilListAdd(&modbusS_RTU, &regCoilItem1)==FALSE){
		return FALSE;
	}
	return TRUE;
}
/*�û����Ը���ĳ����ַ��ֵ*/
void MDS_RTU_UserUpdate(void){
	uint16 temp=0xff;
	uint16 temp1[]={1,2,3,4,5};
	MDS_RTU_WriteBits(&modbusS_RTU,1,5,&temp);
	MDS_RTU_WriteReg(&modbusS_RTU,11, temp);
	MDS_RTU_WriteRegs(&modbusS_RTU,5,5, temp1,0);
}

void MDS_RTU_Poll(void){
	MDS_RTU_Loop(&modbusS_RTU);
	MDS_RTU_UserUpdate();
}

