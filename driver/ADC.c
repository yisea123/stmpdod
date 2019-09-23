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
static u16 start = 0;
static u16 end = 0;
static u16 electricCnt = 0;
static u8 isFirst = 1;
static u16 evtTime = 0;
static u32 Ev = 0 ;

void BeginRefresh(u16* peakvalue)
{
	os_sem_init(refresh_sem, 0);//信号量为零阻塞 time
	ppvalue = peakvalue;//需要处理的变量
}

void WaitRefresh(void)
{
	os_sem_wait(refresh_sem, 0xFFFF);//型号量约束
}

void Tim4_Init(){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitStructure.TIM_Period = 8400 - 1; 	//1s
	TIM_TimeBaseInitStructure.TIM_Prescaler = 84 - 1;//1us
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	TIM_Cmd(TIM4, ENABLE);//开始计时
}

void InitADC(void)
{
	{
		DMA_InitTypeDef  DMA_InitStructure;
		DMA_DeInit(DMA2_Stream0);//按默认值初始化一些寄存器值
		DMA_InitStructure.DMA_Channel = DMA_Channel_0;//选择通道零
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (ADC1->DR);//外设起始
		DMA_InitStructure.DMA_Memory0BaseAddr = (u32)(ADPoint[0]);//内存起始
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到内存
		DMA_InitStructure.DMA_BufferSize = ANALYSENUM ;//一次传输35个
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//选用的通道有多个外设连接才要设置enable
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置内存递增方式，用于数组的时候。
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//半字传输
		DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;//两边要对齐
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//不断传输，循环模式
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;//禁止字节拼接
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//突发模式
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//其中CPU可以获得总线
		DMA_DoubleBufferModeConfig(DMA2_Stream0, (uint32_t)(ADPoint[1]), DMA_Memory_0);//dma双缓存模式，cpu在使用这一片内存时，缓存到这个内存。cpu处理数据与传输数据不打扰、
		DMA_DoubleBufferModeCmd(DMA2_Stream0, ENABLE);//启动双缓存

		DMA_Init(DMA2_Stream0, &DMA_InitStructure);
		DMA_Cmd(DMA2_Stream0, ENABLE);
		DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);//dma2通道零 传输完成中断。
	}
	{
		ADC_InitTypeDef ADC_InitStructure;
		ADC_CommonInitTypeDef ADC_CommonInitStructure;
		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; //独立模式
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//四分屏 84/4 = 21M 因为在这个电压下dma是最大30m时钟
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//dma先失能
		ADC_CommonInit(&ADC_CommonInitStructure);//初始化

		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;//扫描模式
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//启动连续转换
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//使用软件触发
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
		ADC_InitStructure.ADC_NbrOfConversion = 1;//使用1通道,规则通道为1

		ADC_Init(ADC1, &ADC_InitStructure);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);//规则通道，3.0周期

		ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);//源数据变化时开启DMA传输
		ADC_DMACmd(ADC1, ENABLE);
		ADC_Cmd(ADC1, ENABLE);
		//adc转换存到 DMA，
	}
	DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0);//清除通道标志位
	ADC1->CR2 &= ~(1 << 30);
	ADC1->CR2 |= (1 << 30); //软件开启转换
	
	Tim4_Init();//初始化tim4;
}



/*

撞击和撞击计数：超过门槛并是某一通道获取数据的任何信号称之为一个撞击，所测得的撞击个数，可分为总计数，计数率。

事件计数：产生发射的一次材料局部变化成


*/
void PointsHandle(uint16_t points[ANALYSENUM])//处理函数
{
	uint16_t i;
	uint16_t max = 0, min = 0xffff;
	u32 ev = 0; 
	
	for (i = 0; i < ANALYSENUM; i++) { //计算峰值
		
		if (max < points[i]) {
			max = points[i];
		}
		if (min > points[i]) {
			min = points[i];
		}
	}
	if ((max - min) > config.threshold && ppvalue != 0){ //过滤掉噪音，只有去除噪音才能计算电量。
		
		int k = 0;
		for (i = 0; i < ANALYSENUM; i++) { //计算峰值
			ev += points[i] ; //积分
		}
		Ev += ev;
		
		if(isFirst){
				TIM_SetCounter(TIM4, 0);
				end = start = TIM_GetCounter(TIM4);
				isFirst = 0;
			}else{
				end = TIM_GetCounter(TIM4);
				k = end - start;
				if(k <= config.hdt){//同一个事件
					 evtTime += k;
				   start = end;
					
				}else{//不是同一个事件
					electricCnt ++;
					TIM_SetCounter(TIM4, 0);
					start = end = TIM_GetCounter(TIM4);
			
					evtTime = 0;
				}
			}
	//传递三个值，放电总量 u32 放电总时间 u16，u16放电次数。
			
		ppvalue[cnt++] = max - min; //125次，保存阈值
		if (cnt == 125) {
			//ppvalue = 0;存疑？
			cnt = 0;
			isr_sem_send(refresh_sem);//处理完后唤醒进程
		}
	}

}

void DMA2_Stream0_IRQHandler(void)//中断，超声波的数据采样到内存里面，要处理这些数据。
{
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
	if (DMA_GetCurrentMemoryTarget(DMA2_Stream0) == 1) {//dma访问存储区1，cpu可以访问存储区0
		PointsHandle(ADPoint[0]);//处理数据
	} else if (DMA_GetCurrentMemoryTarget(DMA2_Stream0) == 0) {//相反
		PointsHandle(ADPoint[1]);
	}
}
