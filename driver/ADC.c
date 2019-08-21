#include "std.h"
#include "ADC.h"
#include "driver/uart1.h"
#include "gpio.h"
#include <stdlib.h>
#include "crc16.h"
#include "main.h"
#include "../task_modbus.h"
#include "config.h"
#define ADC_CDR_ADDRESS    ((uint32_t)0x40012308)
/*
ADC 的工作频率21MHz，转换一个点需要15个工作周期，超声波的频率在40KHz左右，所以得出21M/15/40K = 35
*/

static uint16_t ADPoint[2][ANALYSENUM];
static OS_SEM refresh_sem;
uint16_t* ppvalue;
static u16 cnt = 0;

void BeginRefresh(u16* peakvalue)
{
	os_sem_init(refresh_sem, 0);
	ppvalue = peakvalue;
}

void WaitRefresh(void)
{
	os_sem_wait(refresh_sem, 0xFFFF);
}

void InitADC(void)
{
	{
		DMA_InitTypeDef  DMA_InitStructure;
		DMA_DeInit(DMA2_Stream0);
		DMA_InitStructure.DMA_Channel = DMA_Channel_0;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR);
		DMA_InitStructure.DMA_Memory0BaseAddr = (u32)(ADPoint[0]);
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = ANALYSENUM ;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_DoubleBufferModeConfig(DMA2_Stream0, (uint32_t)(ADPoint[1]), DMA_Memory_0);
		DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);

		DMA_Init(DMA2_Stream0, &DMA_InitStructure);
		DMA_Cmd(DMA2_Stream0, ENABLE);
		DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	}
	{
		ADC_InitTypeDef ADC_InitStructure;
		ADC_CommonInitTypeDef ADC_CommonInitStructure;
		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
		ADC_CommonInit(&ADC_CommonInitStructure);

		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion = 1;

		ADC_Init(ADC1, &ADC_InitStructure);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);

		ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
		ADC_DMACmd(ADC1, ENABLE);
		ADC_Cmd(ADC1, ENABLE);
	}
	DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0);
	ADC1->CR2 &= ~(1 << 30);
	ADC1->CR2 |= (1 << 30); //开启转换

}

void PointsHandle(uint16_t points[ANALYSENUM])
{
	uint16_t i;
	uint16_t max = 0, min = 0xffff;
	for (i = 0; i < ANALYSENUM; i++) {               //计算峰值
		if (max < points[i]) {
			max = points[i];
		}
		if (min > points[i]) {
			min = points[i];
		}
	}
	if ((max - min) > config.threshold && ppvalue != 0) {  //过滤掉噪音
		ppvalue[cnt++] = max - min;
		if (cnt == 125) {
			ppvalue = 0;
			cnt = 0;
			isr_sem_send(refresh_sem);
		}
	}
}

void DMA2_Stream0_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
	if (DMA_GetCurrentMemoryTarget(DMA2_Stream0) == 1) {
		PointsHandle(ADPoint[0]);
	} else if (DMA_GetCurrentMemoryTarget(DMA2_Stream0) == 0) {
		PointsHandle(ADPoint[1]);
	}
}
