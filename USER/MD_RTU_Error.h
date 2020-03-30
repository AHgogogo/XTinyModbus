#ifndef _MDM_RTU_ERROR_H__
#define _MDM_RTU_ERROR_H__

typedef enum{
	ERR_VOID=0,					/*��ָ�����*/
	ERR_NONE=1,					/*û�д���*/
	ERR_IDLE,						/*����ÿ鵱ǰ������*/
	ERR_QUEUE,					/*����д�����*/
	ERR_OVER_TIME,			/*��ʱ����*/
	ERR_SEND_FIN,				/*�������*/
	ERR_RW_OV_TIME_ERR,	/*���ͳ�ʱ����*/
	ERR_SLAVE_ADDR,			/*�ӻ���ַ��ƥ��*/
	ERR_DATA_LEN,				/*���ݳ��ȴ���*/
	ERR_DATA_SAVE,			/*���ݱ������*/
	
	/*����Ĵ����Ǵӻ����ص�*/
	ERR_READ_COIL,			/*����Ȧ����*/
	ERR_READ_INPUT,			/*���������*/
	ERR_READ_HOLD_REG,	/*�����ּĴ�������*/
	ERR_READ_INPUT_REG,	/*������Ĵ�������*/
	ERR_WRITE_COIL,			/*д��Ȧ����*/
	ERR_WRITE_REG,			/*д�Ĵ�������*/
	ERR_RW_FIN,					/*����ӻ��յ��������յ���ȷ��Ӧ*/
}MDError;

#endif
