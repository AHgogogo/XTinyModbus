/********************************************************************************
* @File name: MD_RTU_RegCoil.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU ��ɢӳ�����ģ��
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_RegCoil.h"
#include "MD_RTU_Fun.h"
/*********************************����******************************************/

/*******************************************************
*
* Function name :RegCoilListAdd
* Description        :���һ��ӳ�䵽������ӻ�
* Parameter         :
*        @obj       	�ӻ�������������ָ��
*        @pRegCoilItem    ���ĳ�� ӳ��,�μ�[PRegCoilItem]
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 RegCoilListAdd(void* obj,PRegCoilItem pRegCoilItem){
	uint16 i=0;
	PRegCoilItem *pRegCoilList=obj;
	if(!obj){return FALSE;}
	for(i=0;i<REG_COIL_ITEM_NUM;i++){
		if(pRegCoilList[i]==NULL){
			pRegCoilList[i]=pRegCoilItem;
			return TRUE;
		}
	}
	return FALSE;
}
/*******************************************************
*
* Function name :RegCoilListDel
* Description        :ɾ��һ��ӳ�䵽������ӻ�
* Parameter         :
*        @obj       	�ӻ�������������ָ��
*        @pRegCoilItem    ɾ��ĳ��ӳ��,�μ�[PRegCoilItem]
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 RegCoilListDel(void* obj,PRegCoilItem pRegCoilItem){
	uint16 i=0;
	PRegCoilItem *pRegCoilList=obj;
	if(!obj){return FALSE;}
	for(i=0;i<REG_COIL_ITEM_NUM;i++){
		if(pRegCoilList[i]==pRegCoilItem){
			pRegCoilList[i]=NULL;
			return TRUE;
		}
	}
	return FALSE;
}



