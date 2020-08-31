/********************************************************************************
* @File name: MD_RTU_Queue.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU ���й���ģ��
********************************************************************************/
#ifndef _SQQUEUE_H__
#define _SQQUEUE_H__

/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_Type.h"
#include "MD_RTU_Config.h"
/*********************************����******************************************/



/*********************************���Ͷ���**************************************/
typedef uint8 MDQueueDateType;

typedef struct {
	MDQueueDateType data[MD_RTU_QUEUE_SIZE+1];
	int16 front, rear;
	uint16 maxVal;
	uint8 valid;
}*PMDSqQueue, MDSqQueue;
/*********************************����******************************************/

/*********************************��������**************************************/
uint8 MDInitQueue(MDSqQueue *q);
uint16 MDQueueLength(MDSqQueue* q);
void 	MDResetQueue(MDSqQueue* q);
uint8 MDQueueEmpty(MDSqQueue* q);
uint8 MDenQueueH(MDSqQueue* q, MDQueueDateType e);
uint8 MDenQueue(MDSqQueue* q, MDQueueDateType e);
uint8 MDdeQueue(MDSqQueue* q, MDQueueDateType* e);
uint8 MDdeQueueF(MDSqQueue* q, MDQueueDateType *e) ;
uint8 MDgetTailQueue(MDSqQueue* q, MDQueueDateType* e);
/*********************************����******************************************/

#endif

