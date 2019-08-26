
#ifndef __GPIO_H__
#define __GPIO_H__

#include "std.h"
static void UART1_StatusRecv(void)
{
	GPIOA->BSRRH = GPIO_Pin_8;//低电平收 清除 同步模式输出如果置一
}

static void UART1_StatusSend(void)
{
	GPIOA->BSRRL = GPIO_Pin_8;	//高电平发 设置
}
static void HeartLED_On(void)
{
	GPIOB->BSRRL = GPIO_Pin_15;
}

static void HeartLED_Off(void)
{
	GPIOB->BSRRH = GPIO_Pin_15;
}
static void HeartLED_Reverse(void)
{
	if (GPIOB->ODR & GPIO_Pin_15) {
		GPIOB->BSRRH = GPIO_Pin_15;
	} else {
		GPIOB->BSRRL = GPIO_Pin_15;
	}
}
static void WDI_Reverse(void)
{
	if (GPIOC->ODR & GPIO_Pin_0) {//输出数据寄存器
		GPIOC->BSRRH = GPIO_Pin_0;//相应的位置零
	} else {
		GPIOC->BSRRL = GPIO_Pin_0;//对相应的位置1
	}
}
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static __inline void GPIO_Configuration(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	//AD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 ; //启用
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//无
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 485
	// USART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 |  GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//备用
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推免
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//数据收发控制端
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推免
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//A1-A2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推免
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//HLD
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//WDI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
#endif
