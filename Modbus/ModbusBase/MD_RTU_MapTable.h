/********************************************************************************
* @File name: MD_RTU_RegCoil.h
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU ��ɢӳ�����ģ��
********************************************************************************/
#ifndef _MD_RTU_REGCOIL_H__
#define _MD_RTU_REGCOIL_H__
/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_Type.h"
/*********************************����******************************************/

/*********************************�Զ�������************************************/
typedef enum{		
	/*����*/
	COILS_TYPE=1,
	INPUT_TYPE=2,
	HOLD_REGS_TYPE=3,
	INPUT_REGS_TYPE=4
}AddrType;

typedef struct{
	uint16 		modbusAddr;			/*modbus�ĵ�ַ*/
	uint16*		modbusData;			/*�洢������*/
	uint16		modbusDataSize;	/*ӳ���С*/
	AddrType	addrType;				/*��ַ���� �μ�[AddrType]*/
	uint8			devAddr;						/*ָʾ��ӳ����������һ���豸,��ʱֻ�������õ��ñ���,�ӻ�����*/
}*PMapTableItem,MapTableItem;
/*********************************����******************************************/

/*********************************��������************************************/
uint8 MapTableAdd(void* obj,PMapTableItem pMapTableItem,uint16 tabSize);
uint8 MapTableDel(void* obj,PMapTableItem pMapTableItem,uint16 tabSize);
/*********************************����******************************************/

#endif
