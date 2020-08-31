/********************************************************************************
* @File name: MD_RTU_Serial.c
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU �������ģ��
* ��Դ��ַ: https://github.com/lotoohe-space/XTinyModbus
********************************************************************************/

/*********************************ͷ�ļ�����************************************/
#include "MDM_RTU_Serial.h"
#include "MD_RTU_Tool.h"
#include "usart.h"
/*********************************����******************************************/

/*********************************ȫ�ֱ���************************************/
PModbusBase pModbusMBase=NULL;		/*��ǰ���ڵ�Modbus*/
/*********************************����******************************************/

/*********************************��������************************************/

//void MDMTimeHandler100US(void);
/*********************************����******************************************/

/*******************************************************
*
* Function name :MDMInitSerial
* Description        	:Ӳ����ʼ�����������ڴ˳�ʼ������
* Parameter         	:
*        @obj        	��������ָ��    
*        @baud    		������
*        @dataBits    ����λ
*        @stopBit    	ֹͣλ
*        @parity    	��żУ��λ
* Return          		:��
**********************************************************/
void MDMInitSerial(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity){
	pModbusMBase=obj;
	if(obj==NULL){return ;}
	
//	pModbusMBase->mdRTUSendBytesFunction=MDMSerialSendBytes;
//	pModbusMBase->mdRTURecSendConv=MDMSerialSWRecv_Send;
	
	/*Ӳ����ʼ��*/
	uart_init(baud);
}
/*******************************************************
*
* Function name :MDMTimeHandler100US
* Description        :��ʱ���е��øú���
* Parameter         :��
* Return          : ��
**********************************************************/
void MDMTimeHandler100US(void){
	if(pModbusMBase==NULL){return;}
	pModbusMBase->mdRTUTimeHandlerFunction(pModbusMBase );
}
/*******************************************************
*
* Function name :MDMSerialRecvByte
* Description        :bsp�㴮���жϽ��յ����������
* Parameter         :
*        @byte    ���յ�һ�ֽ�
* Return          : ��
**********************************************************/
void MDMSerialRecvByte(uint8 byte){
	if(pModbusMBase==NULL){return;}
	pModbusMBase->mdRTURecByteFunction(pModbusMBase , byte);
}
/*******************************************************
*
* Function name :MDMSerialRecvByte
* Description        :�л����գ����߷���
* Parameter         :
*        @mode    TRUE ���ͣ� FALSE����
* Return          : ��
**********************************************************/
void MDMSerialSWRecv_Send(uint8 mode){
	/*�շ�ת��*/
	/*������дת���Ĵ���*/
	
	/*��ͬ��Ӳ������������ת������Ҫһ����ʱ*/
}
/*******************************************************
*
* Function name :MDMSerialSendBytes
* Description        :���ͺ���
* Parameter         :
*        @bytes    ���͵�����
*        @num    ���Ͷ��ٸ��ֽ�
* Return          : ��
**********************************************************/
void MDMSerialSendBytes(uint8 *bytes,uint16 num){
	/*���������bsp�ķ��ͺ���*/
	uart_send_bytes(bytes,num);
}


