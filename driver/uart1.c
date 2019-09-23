#include "std.h"
#include "main.h"
#include "uart1.h"
#include "gpio.h"
#include "ADC.h"
#include <stdlib.h>
#include "task_modbus.h"
#include "modbus.h"
#include "config.h"
static unsigned char recv_rs485_0[sizeof(MODBUSFRAME)];
static unsigned int buflen_rs485_0;
static unsigned char* send_rs485_0;
static unsigned int sendlen_rs485_0;

void UART1_Init(void)
{
	int baudrate = ModbusReg2Int(config.baudrate);
	{
		USART_InitTypeDef USART_InitStructure;
		USART_InitStructure.USART_BaudRate = baudrate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(USART1, &USART_InitStructure);
	}

	TIM_Cmd(TIM2, DISABLE);
	{

		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		TIM_TimeBaseStructure.TIM_Prescaler = 35 - 1;//3.5个字符
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Period = SystemCoreClock / 2 / baudrate - 1; //
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	}
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	UART1_StatusRecv();
}

void USART1_IRQHandler(void)
{
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, ENABLE);

	if (USART1->SR & 0x20) {
		if (buflen_rs485_0 < sizeof(recv_rs485_0)) {
			recv_rs485_0[buflen_rs485_0++] = USART1->DR;
		} else {
			USART1->DR = USART1->DR;
		}
	} else {
		if (sendlen_rs485_0 > 0) {
			USART1->DR = *send_rs485_0;
			send_rs485_0++;
			sendlen_rs485_0--;
		} else {
			USART1->CR1 &= 0xFF7F;
		}
	}
}

__weak void ISR_UARTRS485_0(uint8_t* ptr)
{
}

void TIM2_IRQHandler(void)
{
	TIM2->CR1 &= (uint16_t)~TIM_CR1_CEN;
	TIM2->SR = (uint16_t)~TIM_IT_Update;

	if (buflen_rs485_0 >= 5) {
		ISR_UARTRS485_0(recv_rs485_0);
	}
	buflen_rs485_0 = 0;
}

void UART1_Send(uint8_t* pFrame, uint16_t len)
{
	UART1_StatusSend();
	send_rs485_0 = pFrame;
	sendlen_rs485_0 = len;
	USART1->CR1 |= 0x80;
	while ((sendlen_rs485_0 > 0) || (USART1->SR & USART_FLAG_TC) == 0) {
		os_tsk_pass();
	}
	USART1->SR &= (uint16_t)~USART_FLAG_TC;
	UART1_StatusRecv();
}
