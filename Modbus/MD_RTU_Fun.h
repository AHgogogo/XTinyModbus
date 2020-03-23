/**
* @file 		MD_RTU_Fun.h
* @brief		��
* @details	��
* @author		zspace
* @date		2020/3/23
* @version	A001
* @par History:  ��       
*/
#ifndef _MD_RTU_FUN_H__
#define _MD_RTU_FUN_H__

#include "MD_RTU_Queue.h"
#include "MD_RTU_RegCoil.h"
#include "MD_RTU_Type.h"

typedef uint8 (*MDS_RTU_ReadDataProcessFunction)(uint16 reg,uint16 regNum,uint8 funCode);
typedef uint8 (*MDS_RTU_WriteDataProcessFunction)(uint16 reg,uint16 regNum,uint8 funCode,void* data,uint8 byteCount);
typedef	void	(*MDS_RTU_TimeHandlerFunction)(void* obj,uint32 times);

typedef void (*MDS_RTU_SendBytesFunction)(uint8 *byte,uint16 num);
typedef void (*MDS_RTU_RecByteFunction)(void* obj, uint8 byte);
typedef void (*MDS_RTU_RecSendConv)(uint8 mode);

#define REG_COIL_ITEM_NUM 20

#define 	MDS_RTU_CMD_SIZE		256			/*����ָ���*/
typedef struct{
	MDSqQueue 											mdSqQueue;													/*���ݽ��ն���*/
	MDSqQueue												mdMsgSqQueue;												/*��Ϣ�������*/
	uint8														salveAddr;													/*�ӻ���ַ*/
	
	uint8														serialReadCache[MDS_RTU_CMD_SIZE];	/*��ָ���ȡ����*/
	uint16													serialReadCount;										/*ָ��ĳ���*/
	
	MDS_RTU_ReadDataProcessFunction 	mdRTUReadDataProcessFunction;			/*�����ݻص�����*/
	MDS_RTU_WriteDataProcessFunction 	mdRUTWriteDataProcessFunction;		/*д���ݻص�����*/
	MDS_RTU_TimeHandlerFunction				mdRTUTimeHandlerFunction;					/*��ʱ���ص�����*/
	
	PRegCoilItem 							pRegCoilList[REG_COIL_ITEM_NUM];					/*�Ĵ���ע���*/
	
	/*���ݷ��ͽ����йصĺ���*/
	MDS_RTU_SendBytesFunction	mdsRTUSendBytesFunction;
	MDS_RTU_RecByteFunction		mdsRTURecByteFunction;
	MDS_RTU_RecSendConv				mdsRTURecSendConv;
	
	/*�ϴν��յ�ʱ��,0xFFFFFFF��ʾδ��ʼ���֡*/
	uint32 lastTimesTick;
	/*��ǰ��ʵʱʱ�䵥λ100US*/
	uint32 timesTick;
	
	/*֡���ʱ��3.5T*/
	uint16 frameIntervalTime;
	
	/*���յ�CRC16*/
	uint16 CRC16Update;
	
	/*���͵�CRC16*/
	uint16 CRC16SendUpdate;
}*PModbusS_RTU,ModbusS_RTU;


#define MDS_RTU_FUN_CODE(a)				(((PModbusS_RTU)(a))->serialReadCache[1])
#define MDS_RTU_CRC16(a)						(((((PModbusS_RTU)(a))->serialReadCache[a->serialReadCount-1])<<8)|\
(((PModbusS_RTU)(a))->serialReadCache[a->serialReadCount]))

#define MDS_RTU_START_REG(a)				(((((PModbusS_RTU)(a))->serialReadCache[2])<<8)|\
(((PModbusS_RTU)(a))->serialReadCache[3]))
#define MDS_RTU_REGS_NUM(a)				(((((PModbusS_RTU)(a))->serialReadCache[4])<<8)|\
(((PModbusS_RTU)(a))->serialReadCache[5]))

#define MDS_RTU_REG_COIL_LIST(a)						((a)->pRegCoilList)
#define MDS_RTU_REG_COIL_ITEM_ADDR(a)				((a)->modbusAddr)
#define MDS_RTU_REG_COIL_ITEM_DATA(a)				((a)->modbusData)
#define MDS_RTU_REG_COIL_ITEM_Data_Size(a)	((a)->modbusDataSize)
#define MDS_RTU_REG_COIL_ITEM_ADDR_TYPE(a)	((a)->addrType)

/*�������������modbus����,��ͬʱ����У��*/
#define MSD_START_SEND(a)		(a)->CRC16SendUpdate=0xFFFF
#define MSD_SEND_BYTE(a,b)	(a)->CRC16SendUpdate=crc16_update((a)->CRC16SendUpdate,(b));\
MDS_RTU_SendByte(a,b)
#define MSD_SEND_END(a)			(a)->mdsRTUSendBytesFunction((uint8*)(&((a)->CRC16SendUpdate)),2)

/*����������Ա�����λ���в���*/
#define MDS_GET_BIT(a,b) (((a)>>(b))&0x1)
#define MDS_SET_BIT(a,b) (a)|=(1<<(b))
#define MDS_CLR_BIT(a,b) (a)&=(~(1<<(b)))

/*����һ�����ֵĸߵ��ֽ�*/
#define MDS_SWAP_HL(a) (a)=((((a)&0xff)<<8)|(((a)>>8)&0xff))

void MDS_RTU_Init(PModbusS_RTU pModbusRTU,uint8 salveAddr,uint32 baud,uint8 dataBits,uint8 stopBit,uint8 parity);
void MDS_RTU_Loop(PModbusS_RTU pModbus_RTU);



#endif
