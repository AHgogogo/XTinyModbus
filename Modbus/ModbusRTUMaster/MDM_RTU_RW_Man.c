#include "MDM_RTU_RW_Man.h"
#include "MDM_RTU_Fun.h"
#include "MD_RTU_Tool.h"

MDM_RW_Ctrl MDM_RW_CtrlList[MDM_RW_CTRL_LIST_SIZE]={0};

static PMDM_RW_Ctrl MDM_RW_CtrlNew(void){
	uint16 i;
	for(i=0;i<MDM_RW_CTRL_LIST_SIZE;i++){
		if(!MD_GET_BIT(MDM_RW_CtrlList[i].flag,0)){
			MD_SET_BIT(MDM_RW_CtrlList[i].flag,0);
			return &(MDM_RW_CtrlList[i]);
		}
	}
	return NULL;
}
static PMDM_RW_Ctrl MDM_RW_CtrlFindByFunAddr(MDMSendReadCallBack cbFun){
	uint16 i;
	for(i=0;i<MDM_RW_CTRL_LIST_SIZE;i++){
		if(MD_GET_BIT(MDM_RW_CtrlList[i].flag,0)){
			if(MDM_RW_CtrlList[i].MDMSendReadFun==cbFun){
				return &(MDM_RW_CtrlList[i]);
			}
		}
	}
	return NULL;
}
/*ɾ��һ��RW�ӿ�*/
void MDM_RW_CtrlDelRW(PMDM_RW_Ctrl pMDM_RW_Ctrl){
	if(pMDM_RW_Ctrl==NULL){return ;}
	pMDM_RW_Ctrl->arg=NULL;
	pMDM_RW_Ctrl->flag=0;
	pMDM_RW_Ctrl->MDMSendReadFun=NULL;
	pMDM_RW_Ctrl->RWCtrlName=NULL;
}
/*���һ����д*/
PMDM_RW_Ctrl MDM_RW_CtrlAddRW(MDMSendReadCallBack cbFun,void *arg,const char *RWCtrlName){
	PMDM_RW_Ctrl pMDM_RW_Ctrl=MDM_RW_CtrlNew();
	if(pMDM_RW_Ctrl==NULL){
		return NULL;
	}
	pMDM_RW_Ctrl->MDMSendReadFun=cbFun;
	pMDM_RW_Ctrl->RWCtrlName=RWCtrlName;
	pMDM_RW_Ctrl->arg=arg;
	return pMDM_RW_Ctrl;
}
/*�����Ƿ���һ�Σ�����ѭ������*/
void MDM_RW_CtrlSetRWOnceFlag(PMDM_RW_Ctrl pMDM_RW_Ctrl,BOOL flag){
	if(pMDM_RW_Ctrl==NULL){return ;}
	if(flag){
		MD_SET_BIT(pMDM_RW_Ctrl->flag,1);
	}else{
		MD_CLR_BIT(pMDM_RW_Ctrl->flag,1);
	}
}
/*��λ�ӻ����߱�־λ*/
void MDM_RW_CtrlResetRetranFlag(PMDM_RW_Ctrl pMDM_RW_Ctrl){
	if(pMDM_RW_Ctrl==NULL){return ;}
	MD_CLR_BIT(pMDM_RW_Ctrl->flag,7);
}

/*******************************************************
*
* Function name :MDM_RW_CtrlLoop
* Description        :�ú�������������շ����ƹ��ܽ���ѭ������
* Parameter         :
*        						��
* Return          : ��
**********************************************************/
void MDM_RW_CtrlLoop(void){
	uint16 i;
	MDM_RW_CtrlErr res;
	for(i=0;i<MDM_RW_CTRL_LIST_SIZE;i++){
		if(MD_GET_BIT(MDM_RW_CtrlList[i].flag,0) &&
			(!MD_GET_BIT(MDM_RW_CtrlList[i].flag,7))/*�ӻ����߻��ߵ��η���������ٱ�����*/
		){
			if(MDM_RW_CtrlList[i].MDMSendReadFun==NULL){
				continue;
			}
			
			res=MDM_RW_CtrlList[i].MDMSendReadFun(MDM_RW_CtrlList[i].arg);/*ѭ������*/
			if(res==RW_ERR){/*����ʧ���򲻷���*/
				MD_SET_BIT(MDM_RW_CtrlList[i].flag,7);
			}
			if(res!=RW_NONE){/*���η���and���ͳɹ�����ʧ��*/
				if(MD_GET_BIT(MDM_RW_CtrlList[i].flag,1)){/*���η���*/
					MD_SET_BIT(MDM_RW_CtrlList[i].flag,7);
				}
			}
		}
	}
}




