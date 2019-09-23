#ifndef _ADC_H
#define _ADC_H
#include "stm32f4xx.h"
#define ANALYSENUM       (35)    //一个周期的点数
#define PERIOD_NUM       (50)
#define ONCESENDPOINT    (ANALYSENUM*PERIOD_NUM)
#define BUF_NUM          (30)
#define ADGAIN  (3.3/4096)




typedef struct {
	uint16_t index;
	uint16_t points[ONCESENDPOINT];
	uint16_t crc;
} EXCEPTDATA;

void BeginRefresh(u16* peakvalue);
void WaitRefresh(void);
extern void InitADC(void);
#endif // _ADC_H
