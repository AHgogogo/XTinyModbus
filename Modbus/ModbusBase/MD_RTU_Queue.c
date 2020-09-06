/********************************************************************************
* @File name: MD_RTU_Queue.c
* @Author: zspace
* @Emial: 1358745329@qq.com
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU ���й���ģ��
* ��Դ��ַ: https://github.com/lotoohe-space/XTinyModbus
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_Queue.h"
/*********************************����******************************************/

/*******************************************************
*
* Function name: MDInitQueue
* Description        :��ʼ��һ������
* Parameter         :
*        @q        ���нṹ��ָ��   
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDInitQueue(MDSqQueue *q){
//	MDQueueDateType* data;
	if (q == NULL) { return FALSE; }
	q->maxVal = MD_RTU_QUEUE_SIZE+1;
	q->front = q->rear = 0;
	q->valid = TRUE;
	return TRUE;
}
/*******************************************************
*
* Function name :MDQueueLength
* Description        :��ȡ���������ݵĳ���
* Parameter         :
*        @q        ���нṹ��ָ�� 
* Return          : ���������ݵĳ���
**********************************************************/
uint16 MDQueueLength(MDSqQueue* q) {
	if (q == NULL) { return 0; }
	return (q->rear - q->front + q->maxVal) % q->maxVal;
}
/*******************************************************
*
* Function name: MDResetQueue
* Description        :���и�λ �����㣩
* Parameter         :
*        @q        ���нṹ��ָ��   
* Return          : ��
**********************************************************/
void MDResetQueue(MDSqQueue* q) {
	if (q == NULL) { return ; }
	q->front = q->rear = 0;
}
/*******************************************************
*
* Function name: MDQueueEmpty
* Description        :�����Ƿ�Ϊ��
* Parameter         :
*        @q        ���нṹ��ָ��   
* Return          : TRUE �� , FALSE ����
**********************************************************/
uint8 MDQueueEmpty(MDSqQueue* q) {
	if (q == NULL) { return 1; }
	return (q->front == q->rear);
}
/*******************************************************
*
* Function name: MDenQueue
* Description        :����β������
* Parameter         :
*        @q        ���нṹ��ָ��   
*        @e        �����Ԫ�� 
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDenQueue(MDSqQueue* q, MDQueueDateType e) {
	if ((q->rear + 1) % q->maxVal == q->front) {
		return FALSE;
	}
	q->rear = (q->rear + 1) % q->maxVal;
	q->data[q->rear] = e;
	return TRUE;
}
/*******************************************************
*
* Function name: MDenQueueH
* Description        :����ͷ������
* Parameter         :
*        @q        ���нṹ��ָ��   
*        @e        �����Ԫ�� 
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDenQueueH(MDSqQueue* q, MDQueueDateType e){		
	if((q->front - 1 + q->maxVal) % q->maxVal == q->rear)
			return FALSE;
	
	q->data[q->front] = e;
	q->front = (q->front - 1 + q->maxVal) % q->maxVal;
	return TRUE;
}
/*******************************************************
*
* Function name: MDdeQueue
* Description        :����ͷ��������
* Parameter         :
*        @q        ���нṹ��ָ��   
*        @e        ��ȡ����Ԫ�� 
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDdeQueue(MDSqQueue* q, MDQueueDateType *e) {
	if (q->front == q->rear) { /*���ˣ��򷵻ش���*/
		return FALSE;
	}
	q->front = (q->front + 1) % q->maxVal;
	*e = q->data[q->front];
	return TRUE;
}
/*******************************************************
*
* Function name: MDdeQueueF
* Description        :����β��������
* Parameter         :
*        @q        ���нṹ��ָ��   
*        @e        ��ȡ����Ԫ�� 
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDdeQueueF(MDSqQueue* q, MDQueueDateType *e) {
	if(q->front == q->rear){
			 return FALSE;	
	}
	*e = q->data[q->rear];
	q->rear = (q->rear - 1 + q->maxVal) % q->maxVal;
	return TRUE;
}
/*******************************************************
*
* Function name: MDgetTailQueue
* Description        :��ȡ����β��Ԫ�أ���Ӱ����У�ֻȡԪ��
* Parameter         :
*        @q        ���нṹ��ָ��   
*        @e        ��ȡ����Ԫ�� 
* Return          : TRUE success , FALSE fail
**********************************************************/
uint8 MDgetTailQueue(MDSqQueue* q, MDQueueDateType* e) {
	if (q->front == q->rear) { /*���ˣ��򷵻ش���*/
		return FALSE;
	}
	*e = q->data[(q->front + 1) % q->maxVal];
	return TRUE;
}


