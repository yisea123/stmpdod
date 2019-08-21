#ifndef __UPDATE_H__
#define __UPDATE_H__

typedef struct {
	uint32_t addr;
	uint32_t len;
	uint32_t data[59];
	//uint32_t data[56];
} BINDATA;

extern void UpdateBinData(BINDATA* bindata);
extern void EraseBinArea(uint32_t size);
extern __declspec(noreturn) void __svc(1) LoadUpdate(void);
extern uint16_t  Addr2Sector(uint32_t addr);
#endif
