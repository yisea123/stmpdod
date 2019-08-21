#include "std.h"
#include "update.h"
#include "flash_if.h"
#include "gpio.h"

extern uint8_t Load$$LR$$LR_IROM1$$Base[];
extern uint8_t Load$$LR$$LR_IROM1$$Length[];

#define FLASH_ADDR_40000		0x08040000
#define FLASH_Sector1_SIZE   0x4000
#define FLASH_Sector2_SIZE   0x20000


#define SECTOR_MASK               ((uint32_t)0xFFFFFF07)

#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000)
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000)
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000)
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000)
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000)
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000)
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000)
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000)
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000)
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000)
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000)
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000)

uint16_t  Addr2Sector(uint32_t addr)
{
	if (addr < ADDR_FLASH_SECTOR_1) {
		return FLASH_Sector_0;
	} else if (addr < ADDR_FLASH_SECTOR_2) {
		return FLASH_Sector_1;
	} else if (addr < ADDR_FLASH_SECTOR_3) {
		return FLASH_Sector_2;
	} else if (addr < ADDR_FLASH_SECTOR_4) {
		return FLASH_Sector_3;
	} else if (addr < ADDR_FLASH_SECTOR_5) {
		return FLASH_Sector_4;
	} else if (addr < ADDR_FLASH_SECTOR_6) {
		return FLASH_Sector_5;
	} else if (addr < ADDR_FLASH_SECTOR_7) {
		return FLASH_Sector_6;
	} else if (addr < ADDR_FLASH_SECTOR_8) {
		return FLASH_Sector_7;
	} else if (addr < ADDR_FLASH_SECTOR_9) {
		return FLASH_Sector_8;
	} else if (addr < ADDR_FLASH_SECTOR_10) {
		return FLASH_Sector_9;
	} else if (addr < ADDR_FLASH_SECTOR_11) {
		return FLASH_Sector_10;
	}
	return FLASH_Sector_11;
}


void UpdateBinData(BINDATA* bindata)
{
	uint32_t i;
	FLASH_Unlock();
	for (i = 0; i < bindata->len; i++) {
		FLASH_ProgramWord(FLASH_ADDR_40000 + bindata->addr + i * 4, bindata->data[i]);
	}
	FLASH_Lock();
}
void EraseBinArea(uint32_t size)
{
	uint32_t addr;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	for (addr = FLASH_ADDR_40000; addr < FLASH_ADDR_40000 + size; addr += FLASH_Sector2_SIZE) {
		FLASH_EraseSector(Addr2Sector(addr), VoltageRange_3);
	}
	FLASH_Lock();

}
__declspec(noreturn) void __SVC_1(void)
{
	typedef void (*UPDATE_START)(void);
	UPDATE_START addr = (UPDATE_START)((*(uint32_t*)(FLASH_ADDR_40000 + 13 * 4)) - (uint32_t)Load$$LR$$LR_IROM1$$Base + FLASH_ADDR_40000);
	__disable_irq();
	__disable_fault_irq();
	SCB->VTOR = FLASH_ADDR_40000;
	addr();
}

void UpdateStart(void)
{
	uint32_t addr;
	uint32_t endaddr = (uint32_t)Load$$LR$$LR_IROM1$$Base + (uint32_t)Load$$LR$$LR_IROM1$$Length;
	FLASH_Unlock();
	for (addr = (uint32_t)Load$$LR$$LR_IROM1$$Base; addr < endaddr; addr += FLASH_Sector1_SIZE) {
		WDI_Reverse();
		HeartLED_Reverse();
		FLASH_EraseSector(Addr2Sector(addr), VoltageRange_3);
	}

	for (addr = (uint32_t)Load$$LR$$LR_IROM1$$Base; addr < endaddr; addr += 4) {
		WDI_Reverse();
		FLASH_ProgramWord(addr, *(__IO uint32_t*)(FLASH_ADDR_40000 - (uint32_t)Load$$LR$$LR_IROM1$$Base + addr));
	}
	FLASH_Lock();

	for (addr = 0; addr < 100; addr++) {
		for (endaddr = 0; endaddr < 100000; endaddr++);
		WDI_Reverse();
		HeartLED_Reverse();
	}

	Reboot();

	while (1);
}
