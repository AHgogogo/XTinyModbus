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
	uint16*			modbusData;		/*�洢������*/
	uint16		modbusDataSize;	/*�μ�[AddrType]*/
	AddrType	addrType;				/*��ַ����*/
}*PRegCoilItem,RegCoilItem;
/*********************************����******************************************/

/*********************************��������************************************/
uint8 RegCoilListAdd(void* obj,PRegCoilItem pRegCoilItem,uint16 tabSize);
uint8 RegCoilListDel(void* obj,PRegCoilItem pRegCoilItem,uint16 tabSize);
/*********************************����******************************************/

#endif
