#ifndef _MDM_RTU_SERIAL_H__
#define _MDM_RTU_SERIAL_H__

#include "MD_RTU_Type.h"

/*Ӳ����ʼ�����������ڴ˳�ʼ������*/
void MDMInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity);
/*bsp���жϽ��յ����������*/
void MDMSerialRecvByte(uint8 byte);
/*��ʱ���е��øú���*/
void MDMTimeHandler100US(uint32 times);

#endif
