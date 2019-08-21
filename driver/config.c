#include "config.h"
#include "modbus.h"
#include "crc16.h"
#include <stdlib.h>
#include "std.h"

#define FLASH_SAVE_ADDR 0x080C0000

CONFIG config;

static void Config_Default()
{
	config.threshold = 1000;//峰峰值的阈值
	ModbusI322Reg(config.baudrate, 9600);
	config.slaveaddr = 1;

	memcpy((void*)&modbusreg_2, (void*)&config, sizeof(modbusreg_2));
	SaveConfig2Flash();
}

void Config_Init()
{
	uint16_t crc;
	uint16_t* pData = (uint16_t*)&config ;
	uint16_t nLen = sizeof(config) / 2;
	uint16_t i = 0;
	while (nLen > 0) {
		*pData = *((uint16_t*)(FLASH_SAVE_ADDR + i));
		pData++;
		i += 2;
		nLen--;
	}
	crc = CRC16((unsigned char*)&config, sizeof(config) - sizeof(config.crc));

	if (crc == config.crc) {
		memcpy((void*)&modbusreg_2, (void*)&config, sizeof(modbusreg_2));
	} else {
		Config_Default();
	}
}

void SaveConfig2Flash()
{
	uint16_t i = 0;
	uint16_t* pData = (uint16_t*)&config;
	uint16_t nlen = sizeof(CONFIG) / 2;
	config.crc = CRC16((unsigned char*)&config, sizeof(CONFIG) - sizeof(config.crc));
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	FLASH_EraseSector(Addr2Sector(FLASH_SAVE_ADDR), VoltageRange_3);
	while (nlen > 0) {
		FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + i, *pData);
		pData++;
		i += 2;
		nlen--;
	}
	FLASH_Lock();
}
