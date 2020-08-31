/********************************************************************************
* @File name: MD_RTU_Serial.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU �����й�ģ��
********************************************************************************/
#ifndef _MD_RTU_SERIAL_H__
#define _MD_RTU_SERIAL_H__

/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_Type.h"
/*********************************����******************************************/

/*********************************��������************************************/
void MDSInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity);
void MDSSerialRecvByte(uint8 byte);
void MDSTimeHandler100US(void);

void MDSSerialSendBytes(uint8 *bytes,uint16 num);
void MDSSerialSWRecv_Send(uint8 mode);
/*********************************����******************************************/

#endif
