#ifndef _MDM_RTU_RW_MAN_H__
#define _MDM_RTU_RW_MAN_H__

#include "MD_RTU_Type.h"
#include "MD_RTU_Config.h"

typedef enum{
	RW_NONE=0,
	RW_ERR,
	RW_OK
}MDM_RW_CtrlErr;

typedef MDM_RW_CtrlErr (*MDMSendReadCallBack)(void*arg);

/*�����ķ�����ģʽ���Ϳ�����*/
typedef struct{
	
	MDMSendReadCallBack MDMSendReadFun;	/*ѭ�����õĶ�д����*/
	void *arg;													/*���ݵĲ���*/
	const char *RWCtrlName;							/*���Ϳ��Ƶ�����*/ 
	uint8	flag;													/*��־ 
																			bit:0 �Ƿ�ʹ�� 
																			bit:1 ���η��� 
																			bit:7 �ӻ����߻��ߵ��η������
																			*/
	
}*PMDM_RW_Ctrl,MDM_RW_Ctrl;

PMDM_RW_Ctrl MDM_RW_CtrlAddRW(MDMSendReadCallBack cbFun,void *arg,const char *RWCtrlName);
void MDM_RW_CtrlSetRWOnceFlag(PMDM_RW_Ctrl pMDM_RW_Ctrl,BOOL flag);
void MDM_RW_CtrlResetRetranFlag(PMDM_RW_Ctrl pMDM_RW_Ctrl);
void MDM_RW_CtrlSetRWOnceFlag(PMDM_RW_Ctrl pMDM_RW_Ctrl,BOOL flag);
void MDM_RW_CtrlLoop(void);

#endif
