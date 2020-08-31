#include "delay.h"
#include "tim3.h"
#include "Sys_Config.h"
#if MD_USD_SALVE
#include "MDS_RTU_APP.h"
#else
#include "MDM_RTU_APP.h"
#endif

int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
#if MD_USD_SALVE
		MDS_RTU_APPInit();
#else
		MDM_RTU_APPInit();
#endif
	TIM3_Int_Init(72-1,100-1);
	while(1){
		#if MD_USD_SALVE
		MDS_RTU_Loop();
		#else
		MDM_RTU_Loop();
		#endif
	}
}





