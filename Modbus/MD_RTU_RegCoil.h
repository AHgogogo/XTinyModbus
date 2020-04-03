/**
* @file 		MD_RTU_RegCoil.h
* @brief		��
* @details	��
* @author		zspace
* @date		2020/3/23
* @version	A001
* @par History:  ��       
*/
#ifndef _MD_RTU_REGCOIL_H__
#define _MD_RTU_REGCOIL_H__
#include "MD_RTU_Type.h"

typedef enum{		
	/*����*/
	COILS_TYPE=1,
	INPUT_TYPE=2,
	HOLD_REGS_TYPE=3,
	INPUT_REGS_TYPE=4
}AddrType;

typedef struct{
	uint16 		modbusAddr;	/*modbus�ĵ�ַ*/
	uint16*			modbusData;	/*�洢������*/
	uint16		modbusDataSize;/*��addrType==BIT_TYPE,Ϊbit������addrType==REG_TYPEʱΪ�Ĵ�������*/
	AddrType	addrType;		/*��ַ����*/
												/*MSB:�ÿ鱻����Ϊʹ�ñ�־λ*/
}*PRegCoilItem,RegCoilItem;


uint8 RegCoilListAdd(void* obj,PRegCoilItem pRegCoilItem);
uint8 RegCoilListDel(void* obj,PRegCoilItem pRegCoilItem);
//#define REGCOIL_ITEM_USED_FLAG_SET(a) MDS_SET_BIT(((PRegCoilItem)(a))->addrType,7)
//#define REGCOIL_ITEM_USED_FLAG_CLR(a) MDS_CLR_BIT(((PRegCoilItem)(a))->addrType,7)
//#define REGCOIL_ITEM_USED_FLAG_GET(a) MDS_GET_BIT(((PRegCoilItem)(a))->addrType,7)

#endif
