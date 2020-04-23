/********************************************************************************
* @File name: MD_RTU_RegCoil.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU ��ɢӳ�����ģ��
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_MapTable.h"
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
uint8 RegCoilListAdd(void* obj,PMapTableItem pMapTableItem,uint16 tabSize){
	uint16 i=0;
	PMapTableItem *pMapTableList=obj;
	if(!obj){return FALSE;}
	for(i=0;i<tabSize;i++){
		if(pMapTableList[i]==NULL){
			pMapTableList[i]=pMapTableItem;
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
uint8 RegCoilListDel(void* obj,PMapTableItem pMapTableItem,uint16 tabSize){
	uint16 i=0;
	PMapTableItem *pMapTableList=obj;
	if(!obj){return FALSE;}
	for(i=0;i<tabSize;i++){
		if(pMapTableList[i]==pMapTableItem){
			pMapTableList[i]=NULL;
			return TRUE;
		}
	}
	return FALSE;
}



