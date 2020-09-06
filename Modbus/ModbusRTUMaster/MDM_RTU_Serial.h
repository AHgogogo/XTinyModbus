/********************************************************************************
* @File name: MD_RTU_Serial.h
* @Author: zspace
* @Emial: 1358745329@qq.com
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU 串口相关模块
********************************************************************************/

#ifndef _MDM_RTU_SERIAL_H__
#define _MDM_RTU_SERIAL_H__

/*********************************头文件包含************************************/
#include "MD_RTU_Type.h"
/*********************************结束******************************************/

/*********************************函数申明************************************/
/*硬件初始化函数，可在此初始化串口*/
void MDMInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity);
/*bsp层中断接收调用这个函数*/
void MDMSerialRecvByte(uint8 byte);
/*定时器中调用该函数*/
void MDMTimeHandler100US(void);

void MDMSerialSendBytes(uint8 *bytes,uint16 num);
void MDMSerialSWRecv_Send(uint8 mode);
/*********************************结束******************************************/

#endif
