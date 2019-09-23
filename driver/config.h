#ifndef _CONFIG_H
#define _CONFIG_H
#include "stm32f4xx_flash.h"

typedef struct {
	uint16_t threshold;//阈值
	uint16_t hdt;//最小放电时间间隔（未知）
	uint16_t slaveaddr;
	uint16_t baudrate[2];
	uint16_t crc;
} CONFIG;

extern CONFIG config;

extern void Config_Init(void);
extern void SaveConfig2Flash(void);

#endif
