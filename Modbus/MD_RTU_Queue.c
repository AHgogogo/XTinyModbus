/**
* @file 		MD_RTU_Queue.c
* @brief		��
* @details	��
* @author		zspace
* @date		2020/3/23
* @version	A001
* @par History:  ��       
*/
#include "MD_RTU_Queue.h"

/*��ʼ������*/
uint8 MDInitQueue(MDSqQueue *q){
//	MDQueueDateType* data;
	if (q == NULL) { return FALSE; }
	q->maxVal = MD_RTU_QUEUE_SIZE+1;
	q->front = q->rear = 0;
	q->valid = TRUE;
	return TRUE;
}
uint16 MDQueueLength(MDSqQueue* q) {
	if (q == NULL) { return 0; }
	return (q->rear - q->front + q->maxVal) % q->maxVal;
}
void MDResetQueue(MDSqQueue* q) {
	if (q == NULL) { return ; }
	q->front = q->rear = 0;
}
uint8 MDQueueEmpty(MDSqQueue* q) {
	if (q == NULL) { return 1; }
	return (q->front == q->rear);
}
/*β��*/
uint8 MDenQueue(MDSqQueue* q, MDQueueDateType e) {
	if ((q->rear + 1) % q->maxVal == q->front) {
		return FALSE;
	}
	q->rear = (q->rear + 1) % q->maxVal;
	q->data[q->rear] = e;
	return TRUE;
}
/*ͷ��*/
uint8 MDenQueueH(MDSqQueue* q, MDQueueDateType e){		
	if((q->front - 1 + q->maxVal) % q->maxVal == q->rear)
			return FALSE;
	
	q->data[q->front] = e;
	q->front = (q->front - 1 + q->maxVal) % q->maxVal;
	return TRUE;
}
/*ͷ��*/
uint8 MDdeQueue(MDSqQueue* q, MDQueueDateType *e) {
	if (q->front == q->rear) { /*���ˣ��򷵻ش���*/
		return FALSE;
	}
	q->front = (q->front + 1) % q->maxVal;
	*e = q->data[q->front];
	return TRUE;
}
/*β��*/
uint8 MDdeQueueF(MDSqQueue* q, MDQueueDateType *e) {
	if(q->front == q->rear){
			 return FALSE;	
	}
	*e = q->data[q->rear];
	q->rear = (q->rear - 1 + q->maxVal) % q->maxVal;
	return TRUE;
}
/*��ȡ��β��Ԫ��*/
uint8 MDgetTailQueue(MDSqQueue* q, MDQueueDateType* e) {
	if (q->front == q->rear) { /*���ˣ��򷵻ش���*/
		return FALSE;
	}
	*e = q->data[(q->front + 1) % q->maxVal];
	return TRUE;
}


