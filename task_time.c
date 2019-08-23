#include "std.h"
#include "main.h"
#include "task_time.h"
#include "ADC.h"
#include "gpio.h"
#include "queue.h"

static uint64_t stk[(1000) / 8];

static void TIM3_Init()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitStructure.TIM_Period = 5000 - 1; 	//500ms 定时500ms
	TIM_TimeBaseInitStructure.TIM_Prescaler = 8400 - 1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void TimeProc(void)
{
	static item_t ppvalue;
	TIM3_Init();
	queue_init(&m_queue);
	BeginRefresh((uint16_t*)&ppvalue); 
	while (1) { //用于填充
		WaitRefresh();//等待信号量大于0，否则会阻塞。
		queue_enqueue(&m_queue, &ppvalue);//加入队列
	}
}

void CreatTaskTime()
{
	os_tsk_create_user(TimeProc, PRIORITY_MEDIUM, stk, sizeof(stk));
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) { 
		WDI_Reverse();
		HeartLED_Reverse();
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
