#ifndef _MD_RTU_REGCOIL_H__
#define _MD_RTU_REGCOIL_H__
#include "MD_RTU_Type.h"

typedef enum{		
	BIT_TYPE=0,
	REG_TYPE=1
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
