/********************************************************************************
* @File name: MD_RTU_Serial_1.c
* @Author: zspace
* @Emial: 1358745329@qq.com
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU �����й�ģ��
* ��Դ��ַ: https://github.com/lotoohe-space/XTinyModbus
********************************************************************************/
/*********************************ͷ�ļ�����************************************/
#include "MDS_RTU_Serial.h"
#include "MDS_RTU_Fun.h"

#include "usart3.h"
/*********************************����******************************************/

/*********************************ȫ�ֱ���************************************/
static PModbusBase pModbusBase=NULL;			/*��ǰ���ڵ�Modbus*/
/*********************************����******************************************/

/*********************************��������************************************/

static void MDSSerialSendBytes_1(uint8 *bytes,uint16 num);
static void MDSSerialSWRecv_Send_1(uint8 mode);
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
void MDSInitSerial_1(void* obj,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity){
	pModbusBase=obj;
	if(obj==NULL){return ;}
	
	pModbusBase->mdRTUSendBytesFunction=MDSSerialSendBytes_1;
	pModbusBase->mdRTURecSendConv=MDSSerialSWRecv_Send_1;
	
	
	/*Ӳ����ʼ��*/
	init_usart3(baud);
}
/*********************************************************
*
* Function name :MDSSerialRecvByte
* Description        :bsp���жϽ��յ����������
* Parameter         :
*        @byte        ���յ���һ���ֽ�    
* Return          : ��
**********************************************************/
void MDSSerialRecvByte_1(uint8 byte){
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
void MDSSerialSWRecv_Send_1(uint8 mode){
	/*�շ�ת��*/
	/*������дת���Ĵ���*/
	RS485_RW_CONV=mode;
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
void MDSSerialSendBytes_1(uint8 *bytes,uint16 num){
	/*���������bsp�ķ��ͺ���*/
	usart3_send_bytes(bytes,num);
}
/*******************************************************
*
* Function name :MDSTimeHandler100US
* Description        :��ʱ���е��øú�������ʱ��λ100US
* Parameter         :��
* Return          : ��
**********************************************************/
void MDSTimeHandler100US_1(void){
	if(pModbusBase==NULL){return;}
	pModbusBase->mdRTUTimeHandlerFunction(pModbusBase);
}
