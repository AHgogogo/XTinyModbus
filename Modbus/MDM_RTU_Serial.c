#include "MDM_RTU_Serial.h"
#include "MD_RTU_Tool.h"
#include "usart.h"

/*��ǰ���ڵ�Modbus*/
PModbusBase pModbusMBase=NULL;

static void MDMSerialSendBytes(uint8 *bytes,uint16 num);
static void MDMSerialSWRecv_Send(uint8 mode);
void MDMTimeHandler100US(uint32 times);

/*Ӳ����ʼ�����������ڴ˳�ʼ������*/
void MDMInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity){
	pModbusMBase=obj;
	if(obj==NULL){return ;}
	
	pModbusMBase->mdRTUSendBytesFunction=MDMSerialSendBytes;
	pModbusMBase->mdRTURecSendConv=MDMSerialSWRecv_Send;
}
/*��ʱ���е��øú���*/
void MDMTimeHandler100US(uint32 times){
	if(pModbusMBase==NULL){return;}
	pModbusMBase->mdRTUTimeHandlerFunction(pModbusMBase ,times);
}
/*bsp���жϽ��յ����������*/
void MDMSerialRecvByte(uint8 byte){
	if(pModbusMBase==NULL){return;}
	pModbusMBase->mdRTURecByteFunction(pModbusMBase , byte);
}

/*�л����գ����߷���*/
static void MDMSerialSWRecv_Send(uint8 mode){
	/*��ʱû��ʵ�ָù���*/
}
/*���ͺ���*/
static void MDMSerialSendBytes(uint8 *bytes,uint16 num){
	/*���������bsp�ķ��ͺ���*/
	uart_send_bytes(bytes,num);
}


