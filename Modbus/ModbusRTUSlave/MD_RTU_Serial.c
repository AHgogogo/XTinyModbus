/********************************************************************************
* @File name: MD_RTU_Serial.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU �����й�ģ��
********************************************************************************/
/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_Serial.h"
#include "MD_RTU_Fun.h"

#include "usart.h"
/*********************************����******************************************/

/*********************************ȫ�ֱ���************************************/
PModbusBase pModbusBase=NULL;			/*��ǰ���ڵ�Modbus*/
/*********************************����******************************************/

/*********************************��������************************************/
void MDSSerialSendBytes(uint8 *bytes,uint16 num);
void MDSSerialSWRecv_Send(uint8 mode);
void MDSTimeHandler100US(void);
/*********************************����******************************************/

/*******************************************************
*
* Function name :MDSInitSerial
* Description        :Ӳ����ʼ�����������ڴ˳�ʼ������
* Parameter         :
*        @obj        �ӻ�����ָ��    
*        @baud    ������
*        @dataBits    ����λ
*        @stopBit    ֹͣλ
*        @parity    ��żУ��λ
* Return          : ��
**********************************************************/
void MDSInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity){
	pModbusBase=obj;
	if(obj==NULL){return ;}
	
	pModbusBase->mdRTUSendBytesFunction=MDSSerialSendBytes;
	pModbusBase->mdRTURecSendConv=MDSSerialSWRecv_Send;
	
	
	/*Ӳ����ʼ��*/
	uart_init(baud);
}
/*******************************************************
*
* Function name :MDSSerialRecvByte
* Description        :bsp���жϽ��յ����������
* Parameter         :
*        @byte        ���յ���һ���ֽ�    
* Return          : ��
**********************************************************/
void MDSSerialRecvByte(uint8 byte){
	if(pModbusBase==NULL){return;}
	pModbusBase->mdRTURecByteFunction(pModbusBase , byte);
}
/*******************************************************
*
* Function name :MDSSerialSWRecv_Send
* Description        :�л����գ����߷���
* Parameter         :
*        @mode        TRUE �� ,FALSE ��
* Return          : ��
**********************************************************/
void MDSSerialSWRecv_Send(uint8 mode){
	/*�շ�ת��*/
	/*������дת���Ĵ���*/
	
	/*��ͬ��Ӳ������������ת������Ҫһ����ʱ*/
}
/*******************************************************
*
* Function name :MDSSerialSendBytes
* Description        :���ͺ���
* Parameter         :
*        @bytes        ���͵�����
*        @num        ���Ͷ����ֽ�
* Return          : ��
**********************************************************/
void MDSSerialSendBytes(uint8 *bytes,uint16 num){
	/*���������bsp�ķ��ͺ���*/
	uart_send_bytes_by_isr(bytes,num);
}
/*******************************************************
*
* Function name :MDSTimeHandler100US
* Description        :��ʱ���е��øú�������ʱ��λ100US
* Parameter         :��
* Return          : ��
**********************************************************/
void MDSTimeHandler100US(void){
	if(pModbusBase==NULL){return;}
	pModbusBase->mdRTUTimeHandlerFunction(pModbusBase);
}
