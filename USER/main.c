#include "delay.h"
#include "tim3.h"

#include "MD_RTU_APP.h"
#include "MDM_RTU_APP.h"

 int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	MDS_RTU_APPInit();
	//MDM_RTU_APPInit();
	TIM3_Int_Init(72-1,100-1);
	while(1){
		MDS_RTU_Loop();
		//MDM_RTU_Loop();
	}
}





