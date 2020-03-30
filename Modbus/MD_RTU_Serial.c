/**
* @file 		MD_RTU_Serial.c
* @brief		��
* @details	��
* @author		zspace
* @date		2020/3/23
* @version	A001
* @par History:  ��       
*/
#include "MD_RTU_Serial.h"

#include "MD_RTU_Fun.h"
#include "usart.h"

/*��ǰ���ڵ�Modbus*/
PModbusBase pModbusBase=NULL;

void MDSSerialSendBytes(uint8 *bytes,uint16 num);
void MDSSerialSWRecv_Send(uint8 mode);
void MDSTimeHandler100US(uint32 times);

/*Ӳ����ʼ�����������ڴ˳�ʼ������*/
void MDSInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity){
	pModbusBase=obj;
	if(obj==NULL){return ;}
	
	pModbusBase->mdRTUSendBytesFunction=MDSSerialSendBytes;
	pModbusBase->mdRTURecSendConv=MDSSerialSWRecv_Send;
}

/*bsp���жϽ��յ����������*/
void MDSSerialRecvByte(uint8 byte){
	if(pModbusBase==NULL){return;}
	pModbusBase->mdRTURecByteFunction(pModbusBase , byte);
}

/*�л����գ����߷���*/
void MDSSerialSWRecv_Send(uint8 mode){
	/*��ʱû��ʵ�ָù���*/
}
/*���ͺ���*/
void MDSSerialSendBytes(uint8 *bytes,uint16 num){
	/*���������bsp�ķ��ͺ���*/
	uart_send_bytes(bytes,num);
}
/*��ʱ���е��øú���*/
void MDSTimeHandler100US(uint32 times){
	if(pModbusBase==NULL){return;}
	pModbusBase->mdRTUTimeHandlerFunction(pModbusBase ,times);
}
