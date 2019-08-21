#ifndef _STDAFX_H
#define _STDAFX_H

#if 0
#define __irq
#define at
#define __weak
#define __nop
#define VERSION A
#endif
/*
#ifndef STM32F10X_XL
#define STM32F10X_XL
#endif // STM32F10X_XL
#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif // USE_STDPERIPH_DRIVER
#ifndef HSE_VALUE
#define HSE_VALUE 64000000
#endif // HSE_VALUE*/

#include <RTL.h>

#include <string.h>
#include "stm32f4xx.h"
//#include <system_stm32f4xx.h>
//#include "stm32f4xx_conf.h"

#define LOWORD(a)	((uint16_t)(a))
#define HIWORD(a)	((uint16_t)(a>>16))

#define LOBYTE(a)	((uint8_t)(a))
#define HIBYTE(a)	((uint8_t)(a>>8))

#define MAKEWORD(a, b)	((((uint16_t)b)<<8) + a)
#define MAKELONG(a, b)	((((uint32_t)b)<<16) + a)

#define DEVICEID (*((uint32_t*)0x1FFFF7E8))

extern U16 os_time;

extern void delay_us(int cnt);
extern void Reboot(void);

#endif
