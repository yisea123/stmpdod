#ifndef __DG_CORE_H__
#define __DG_CORE_H__

#define MODBUS_TIMER	1

#define NPOINT 				64			 		/* FFT¼ÆËãµãÊý */

#define PRIORITY_LOWEST		1
#define PRIORITY_LOW		2
#define PRIORITY_MEDIUM		3
#define PRIORITY_HIGH		4
#define PRIORITY_HIGHEST	5

#define EVENT_I2C1			(1<<15)
#define EVENT_I2C2			(1<<14)
#define EVENT_AT45DB_DMA	(1<<13)
#define EVENT_W5200_DMA		(1<<12)

#define RUNEVT_STARTUP			1
#define RUNEVT_REBOOT			3
#define RUNEVT_UPGRADE			4
#define RUNEVT_I2C_TIMEOUT		13
#define RUNEVT_TSK_1S_TIMEOUT	14
#define RUNEVT_MODBUS_TIMEOUT	15

#define RUNEVT_HSDC_TIMEOUT		17
typedef struct {
	uint8_t* pData;
	uint32_t datalen;
} SENDDATA;

extern void IWDG_Init(uint8_t Prescaler, uint16_t Reload);
#endif
