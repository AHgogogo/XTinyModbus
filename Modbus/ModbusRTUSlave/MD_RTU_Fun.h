/********************************************************************************
* @File name: MD_RTU_Fun.h
* @Author: zspace
* @Version: 1.0
* @Date: 2020-4-10
* @Description: Modbus RTU Slave�ӻ����չ���ģ�顣
********************************************************************************/

#ifndef _MD_RTU_FUN_H__
#define _MD_RTU_FUN_H__
/*********************************ͷ�ļ�����************************************/
#include "MD_RTU_Queue.h"
#include "MD_RTU_RegCoil.h"
#include "MD_RTU_Type.h"
#include "MD_RTU_Tool.h"
/*********************************����******************************************/

/*********************************���ò���***************************************/
#define 	REG_COIL_ITEM_NUM 	20			/*��ɢӳ���������*/
#define 	MDS_RTU_CMD_SIZE		256			/*����ָ���*/
/*********************************����******************************************/


/*********************************������������***********************************/
typedef void (*MDSWriteFunciton)(void* obj,uint16 modbusAddr,uint16 wLen,AddrType addrType);

typedef struct{	
	ModbusBase											modbusBase;													/*�̳�modbusBase*/
	MDSWriteFunciton								mdsWriteFun;												/*����д�ص�����*/
	MDSqQueue 											mdSqQueue;													/*���ݽ��ն���*/
	MDSqQueue												mdMsgSqQueue;												/*��Ϣ�������*/
	uint8														salveAddr;													/*�ӻ���ַ*/
	
	uint8														serialReadCache[MDS_RTU_CMD_SIZE];	/*��ָ���ȡ����*/
	uint16													serialReadCount;										/*ָ��ĳ���*/

	PRegCoilItem 										pRegCoilList[REG_COIL_ITEM_NUM];					/*�Ĵ���ע���*/

	/*�ϴν��յ�ʱ��,0xFFFFFFF��ʾδ��ʼ���֡*/
	uint32 lastTimesTick;
	/*��ǰ��ʵʱʱ�䵥λ100US*/
	uint32 timesTick;
	
	/*�ϴη��͵�ʱ��*/
	uint32 lastSendTimes;
	
	/*֡���ʱ��3.5T*/
	uint16 frameIntervalTime;
	
	/*���յ�CRC16*/
	uint16 CRC16Update;
	
	/*���͵�CRC16*/
	uint16 CRC16SendUpdate;
}*PModbusS_RTU,ModbusS_RTU;

/*�쳣��*/
typedef enum{
	READ_COIL_ANL=0x81,
	READ_INPUT_ANL=0x82,
	READ_HOLD_REGS=0x83,
	READ_INPUT_REGS=0X84,
	WRITE_SIN_COIL=0X85,
	WRITE_SIN_REG=0x86,
	WRITE_COILS=0X8F,
	WRITE_REGS=0X90
}ANLCode;

/*������*/
typedef enum{
	ILLEGAL_FUN=0x01,				/*��δ֧��*/
	ILLEGAL_DAT_ADDR=0x2,
	ILLEGAL_DAT_VAL=0x3,
	SLAVE_DEV_FAILURE=0x4,	/*��δ֧��*/
	/*���滹�д����룬������ʱδʹ��*/
}ErrorCode;
/*********************************����******************************************/

/*********************************�궨��****************************************/
#define MDS_RTU_FUN_CODE(a)					(((PModbusS_RTU)(a))->serialReadCache[1])
#define MDS_RTU_CRC16(a)						(((((PModbusS_RTU)(a))->serialReadCache[a->serialReadCount-1])<<8)|\
(((PModbusS_RTU)(a))->serialReadCache[a->serialReadCount]))

#define MDS_RTU_START_REG(a)				(((((PModbusS_RTU)(a))->serialReadCache[2])<<8)|\
(((PModbusS_RTU)(a))->serialReadCache[3]))
#define MDS_RTU_REGS_NUM(a)					(((((PModbusS_RTU)(a))->serialReadCache[4])<<8)|\
(((PModbusS_RTU)(a))->serialReadCache[5]))
#define MDS_RTU_BYTES_NUM(a)				((a)->serialReadCache[6])

/*�������������modbus����,��ͬʱ����У��*/
#define MSD_START_SEND(a)		(a)->CRC16SendUpdate=0xFFFF
#define MSD_SEND_BYTE(a,b)	(a)->CRC16SendUpdate=MD_CRC16Update((a)->CRC16SendUpdate,(b));\
MDS_RTU_SendByte(a,b)
#define MSD_SEND_END(a)			(TO_MDBase(a))->mdRTUSendBytesFunction((uint8*)(&((a)->CRC16SendUpdate)),2)
/*********************************����******************************************/


/*********************************��������************************************/
void MDS_RTU_Init(PModbusS_RTU pModbusRTU,MD_RTU_SerialInit mdRTUSerialInitFun,uint8 salveAddr,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity);
void MDS_RTU_SetWriteListenFun(PModbusS_RTU pModbus_RTU,MDSWriteFunciton wFun);
void MDS_RTU_Process(PModbusS_RTU pModbus_RTU);
/*********************************����******************************************/

#endif
