#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "tim3.h"

#include "MD_RTU_APP.h"
#include "MDM_RTU_APP.h"
#include "w25qxx.h"
#include "touch.h"
#include "GBK_LibDrive.h"	

#include "bsp_amg88xx.h"

#include <stdlib.h>



 int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	//uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	 
 	LED_Init();			     //LED�˿ڳ�ʼ��
	LCD_Init();          //Һ������ʼ��
  W25QXX_Init();			//W25QXX��ʼ��

  POINT_COLOR=RED; 
	
	while(W25QXX_ReadID()!=W25Q16)								//��ⲻ��W25QXX
	{
		LCD_ShowString(30,150,200,16,16,RED,"W25Q16 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,150,200,16,16,RED,"Please Check!      ");
		delay_ms(500);
		LED1=!LED1;		//DS0��˸
	}
	LCD_ShowString(30,150,200,16,16,RED,"W25Q16 Ready!"); 
	
  GBK_Lib_Init();       //Ӳ��GBK�ֿ��ʼ��--(���ʹ�ò����ֿ��Һ�����汾���˴��������Σ������ֿ��ʼ���� 
	
	KEY_Init();	//������ʼ��
	 
 	tp_dev.init();//��������ʼ��
	
	amg88xx_init();
	
  LCD_Clear(WHITE);//�����Ļ

	MDS_RTU_APPInit();
	//MDM_RTU_APPInit();
	TIM3_Int_Init(72-1,100-1);
	while(1){
		MDS_RTU_Loop();
		//MDM_RTU_Loop();
		//delay_ms(50);
	}
}
////�ŵ��������ĳ�ʼ���г�ʼ��
//void Timer1CountInitial(void)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	TIM_DeInit(TIM1);	/*��λTIM1��ʱ��*/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	/*��ʱ��*/
//	TIM_TimeBaseStructure.TIM_Period = 100;	/*ʱ�ӵδ�Ĵ����������ж�������1ms�ж�һ��*/     
//	TIM_TimeBaseStructure.TIM_Prescaler = 720-1;	/* ��Ƶ720*/       
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  	 
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	/*�����������ϼ���*/
//	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
//	/* Clear TIM1 update pending flag  ���TIM1����жϱ�־]  */
//	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
//	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;	/*����ж�*/
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;  
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);	/* Enable TIM1 Update interrupt TIM1����ж�����*/
//	TIM1->CR1 |= TIM_CR1_CEN;	/*����Timer1����*/
//}


//void TIM1_UP_IRQHandler(void)
//{        
//	//TIM_TimeBaseStructure.TIM_Period = 100-1;//�Զ���װֵ����ʱ���жϵ�����Ϊ100ms��
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
//	{  
//		sys_tick++;
//		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
//	}
//}




