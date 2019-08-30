#include "std.h"
#include "main.h"
#include "modbus.h"
#include "crc16.h"

DEVICE_REG_0 modbusreg_0;//这里就是主要的传输部分。
DEVICE_REG_1 modbusreg_1;
DEVICE_REG_2 modbusreg_2;
DEVICE_REG_3 modbusreg_3;

static uint16_t* const _modbusreg_0 = (uint16_t*)& modbusreg_0;
static uint16_t* const _modbusreg_1 = (uint16_t*)& modbusreg_1;
static uint16_t* const _modbusreg_2 = (uint16_t*)& modbusreg_2;
static uint16_t* const _modbusreg_3 = (uint16_t*)& modbusreg_3;

MODBUSFRAME frame;
//传输协议 modbus -rtu实现

static uint16_t* AccessAddr(uint16_t addr) //主机要读取从机的地址，验证地址
{
	if (addr >= 1024 && addr < 1146) {
		return _modbusreg_1 + addr - 1024;
	} else if (addr >= 2048 && addr < 2054) {
		return _modbusreg_2 + addr - 2048;
	} else {
		return 0;
	}
}

uint16_t ModbusMasterFrameLength(MODBUSFRAME* pFrame)
{
	uint16_t len = 0;
	switch (pFrame->header.func) {
		case 3:
			len = 8;
			break;
		case 16:
			len = 9 + pFrame->master16.len;
			break;
	}

	return len;
}

uint16_t ModbusSlaveFrameLength(MODBUSFRAME* pFrame)
{
	uint16_t len = 0;
	switch (pFrame->header.func) {
		case 3:
			len = 5 + pFrame->slave03.len;
			break;
		case 16:
			len = 8;
			break;
	}

	return len;
}

static void ModbusMakeCRC(MODBUSFRAME* pFrame)
{
	uint16_t len = ModbusSlaveFrameLength(pFrame);
	uint16_t crc = CRC16((uint8_t*)pFrame, len - 2);
	((u8*)pFrame)[len - 2] = crc >> 8;
	((u8*)pFrame)[len - 1] = crc;
}

void ModbusMakeMasterCRC(MODBUSFRAME* pFrame)//主机算crc校验
{
	uint16_t len = ModbusMasterFrameLength(pFrame);
	uint16_t crc = CRC16((uint8_t*)pFrame, len - 2);
	((u8*)pFrame)[len - 2] = crc >> 8;
	((u8*)pFrame)[len - 1] = crc;
}


uint8_t ModbusMasterCheckCRC(MODBUSFRAME* pFrame)
{
	uint16_t len = ModbusMasterFrameLength(pFrame);
	if (len > 2) {
		uint16_t crc = CRC16((uint8_t*)pFrame, len - 2);//在算一遍
		return crc == MAKEWORD(((u8*)pFrame)[len - 1], ((u8*)pFrame)[len - 2]);//与之前传输前算的是否一致
	} else {
		return 0;
	}
}

uint8_t ModbusSlaveCheckCRC(MODBUSFRAME* pFrame)
{
	uint16_t len = ModbusSlaveFrameLength(pFrame);
	if (len > 2) {
		uint16_t crc = CRC16((uint8_t*)pFrame, len - 2);
		return crc == MAKEWORD(((u8*)pFrame)[len - 1], ((u8*)pFrame)[len - 2]);
	} else {
		return 0;
	}
}
//-------------------------
void ModbusReadRegs(MODBUSFRAME* pFrame)//读取寄存器的数据（从机）
{
	uint16_t addrbegin, addrend;
	int dataidx = 0;

	addrbegin = MAKEWORD(pFrame->master03.addrlo, pFrame->master03.addrhi);
	addrend = addrbegin + MAKEWORD(pFrame->master03.numlo, pFrame->master03.numhi);
	pFrame->slave03.len = 0;
	while (addrbegin < addrend) {
		uint16_t data;
		if (addrbegin < 189) { //根据不同地址填充数据。
			data = _modbusreg_0[addrbegin];
		} else if (addrbegin >= 2048 && addrbegin < 2054) {
			data = _modbusreg_2[addrbegin - 2048];
		} else if (addrbegin >= 3072 && addrbegin < (3072 + sizeof(DEVICE_REG_3))) {
			data = _modbusreg_3[addrbegin - 3072];
		} else {
			data = 0xFFFF;
		}
		pFrame->slave03.data[dataidx++] = data >> 8; 
		pFrame->slave03.data[dataidx++] = data;
		pFrame->slave03.len += 2;
		addrbegin++;
	}
	ModbusMakeCRC(pFrame);
}

void ModbusWriteRegs(MODBUSFRAME* pFrame)//寄存器的数据（从机）
{
	uint16_t addrbegin, addrend;
	int dataidx = 0;

	addrbegin = MAKEWORD(pFrame->master16.addrlo, pFrame->master16.addrhi);
	addrend = addrbegin + MAKEWORD(pFrame->master16.numlo, pFrame->master16.numhi);

	while (addrbegin < addrend) {
		uint16_t* reg = AccessAddr(addrbegin);//写到内存中某些地方。
		if (reg != 0) {
			*reg = MAKEWORD(pFrame->master16.data[dataidx + 1], pFrame->master16.data[dataidx]);
		}
		addrbegin++;//以字节增加
		dataidx += 2;
	}
	ModbusMakeCRC(pFrame);
}
//-------------------------------------
static void ModbusSetSoftVersion(void)
{
	int i;
//软件版本
	for (i = 0; i < 32; i++) {
		modbusreg_0.soft_mode[i] = 0;
	}
	modbusreg_0.soft_mode[0] = MAKEWORD('D', 'P');
	modbusreg_0.soft_mode[1] = MAKEWORD('D', 'O');
	modbusreg_0.soft_mode[2] = MAKEWORD('0', '-');
	modbusreg_0.soft_mode[3] = MAKEWORD(0, '4');
	modbusreg_0.soft_mode[4] = MAKEWORD(0, 0);
	modbusreg_0.soft_mode[5] = MAKEWORD(0, 0);
	modbusreg_0.soft_mode[6] = MAKEWORD(0, 0);

	for (i = 0; i < 32; i++) {
		modbusreg_0.soft_version[i] = 0;
	}
	modbusreg_0.soft_version[0] = MAKEWORD('.', '1');
	modbusreg_0.soft_version[1] = MAKEWORD('.', '1');
	modbusreg_0.soft_version[2] = MAKEWORD('7', '1');
	modbusreg_0.soft_version[3] = MAKEWORD(0, 0);
}

void ModbusInit(void)
{
	ModbusSetSoftVersion();//初始化
}

uint8_t IsReadPeakVale(uint16_t addrbegin, uint16_t addrend)
{
	if (addrbegin == 64 && addrend == 189) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t IsEraseCMD(uint16_t addrbegin, uint16_t addrend)
{
	if (addrbegin == 1024 && addrend == 1027 && modbusreg_1.cmd == 1) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t IsUpdateBinData(uint16_t addrbegin, uint16_t addrend)
{
	if (addrbegin == 1024 && addrend >= 1028 && modbusreg_1.cmd == 2 && modbusreg_1.length == addrend - 1028) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t IsUpdateCMD(uint16_t addrbegin, uint16_t addrend)
{
	if (addrbegin == 1024 && addrend == 1027 && modbusreg_1.cmd == 3) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t IsRebootCMD(uint16_t addrbegin, uint16_t addrend)
{
	if (addrbegin == 1024 && addrend == 1025 && modbusreg_1.cmd == 4) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t IsConfigWritten(uint16_t addrbegin, uint16_t addrend)
{
	if (addrbegin  < 2051 && addrend > 2048) {
		return 1;
	} else {
		return 0;
	}
}

void GetModbusBinData(BINDATA* bindata)
{
	uint16_t i;
	bindata->addr = MAKELONG(modbusreg_1.addr[0], modbusreg_1.addr[1]);
	bindata->len = modbusreg_1.length / 2;
	for (i = 0; i < bindata->len; i++) {
		bindata->data[i] = MAKELONG(modbusreg_1.data[i * 2], modbusreg_1.data[i * 2 + 1]);
	}
}

__asm void ModbusI322Reg(uint16_t* pReg, int32_t nValue)
{
	STRH     r1, [r0, #0x02]
	ASR      r1, r1, #16
	STRH     r1, [r0, #0x00]
	BX       lr
}

__asm void ModbusFloat2Reg(uint16_t* pReg, float fValue)
{
	STRH     r1, [r0, #0x02] 
	ASR      r1, r1, #16
	STRH     r1, [r0, #0x00]
	BX       lr
}

__asm void ModbusI642Reg(uint16_t* pReg, int64_t nValue)
{
	STRH     r2, [r0, #0x06]
	ASR      r2, r2, #16
	STRH     r2, [r0, #0x04]
	STRH     r3, [r0, #0x02]
	ASR      r3, r3, #16
	STRH     r3, [r0, #0x00]
	BX       lr
}

__asm void ModbusDouble2Reg(uint16_t* pReg, double nValue)
{
	STRH     r2, [r0, #0x06]
	ASR      r2, r2, #16
	STRH     r2, [r0, #0x04]
	STRH     r3, [r0, #0x02]
	ASR      r3, r3, #16
	STRH     r3, [r0, #0x00]
	BX       lr
}

__asm int32_t ModbusReg2Int(uint16_t* pReg)
{
	LDRH     r1, [r0, #0x00]
	LDRH     r0, [r0, #0x02]
	ORR      r0, r1, LSL #16
	BX       lr
}

__asm float ModbusReg2Float(uint16_t* pReg)
{
	LDRH     r1, [r0, #0x00]
	LDRH     r0, [r0, #0x02]
	ORR      r0, r1, LSL #16
	BX       lr
}

__asm double ModbusReg2Double(uint16_t* pReg)
{
	MOV	     r2, r0
	LDRH     r0, [r2, #0x06]
	LDRH     r3, [r2, #0x04]
	ORR      r0, r3, LSL #16
	LDRH     r1, [r2, #0x02]
	LDRH     r3, [r2, #0x00]
	ORR      r1, r3, LSL #16
	BX       lr
}
__asm void ModbusFloat2Byte(uint8_t* pData, float fValue) // R0,R1
{
	STRB     r1, [r0, #0x03]
	ASR      r1, r1, #8
	STRB     r1, [r0, #0x02]
	ASR      r1, r1, #8
	STRB     r1, [r0, #0x01]
	ASR      r1, r1, #8
	STRB     r1, [r0, #0x00]
	BX       lr
}



