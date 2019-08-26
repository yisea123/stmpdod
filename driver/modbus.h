#ifndef __MODBUS_H__
#define __MODBUS_H__

#include "stm32f4xx.h"
#include "update.h"
#include "rtl.h"
#include "ADC.h"

#pragma pack(1) //结构体对齐 一字节对齐，节省空间，如果定义四字节，或者八字节对齐。

//上面设置对齐为一个字节，所以就是char 数组该结构体
typedef union { //覆盖技术
	struct {
		u8 slaveaddr;//从机地址
		u8 func;//功能号
	} header;
	struct {
		u8 slaveaddr;
		u8 func;
		u8 addrhi;//数据起始地址
		u8 addrlo;
		u8 numhi;//读取的数据个数
		u8 numlo;
		u8 crc[2];//crc校验位
	} master03;
	struct {
		u8 slaveaddr;
		u8 func;
		u8 len;
		u8 data[250];
		u8 crc[2];
	} slave03;
	struct {
		u8 slaveaddr;
		u8 func;
		u8 addrhi;
		u8 addrlo;
		u8 numhi;
		u8 numlo;
		u8 len;
		u8 data[246];
		u8 crc[2];
	} master16;//请求控制用
	struct {
		u8 slaveaddr;
		u8 func;
		u8 addrhi;
		u8 addrlo;
		u8 numhi;
		u8 numlo;
		u8 crc[2];
	} slave16;
} MODBUSFRAME;

#pragma pack()

#pragma pack(2)

typedef struct {
	uint16_t soft_mode[32];
	uint16_t soft_version[32];
} DEVICE_REG_0;//关于传输协议约定，版本号等

typedef struct {//升级用的结构
	uint16_t cmd;
	uint16_t addr[2];
	uint16_t length;
	uint16_t data[118];
} DEVICE_REG_1;

typedef struct {//上位机发送约定波特率，也是配置用
	uint16_t threshold;
	uint16_t slaveaddr;
	uint16_t baudrate[2];
} DEVICE_REG_2;

typedef struct {
	uint16_t peakvalue[125];
} DEVICE_REG_3;//关于超声波的数据

#pragma pack()
extern DEVICE_REG_0 modbusreg_0;
extern DEVICE_REG_1 modbusreg_1;//1024
extern DEVICE_REG_2 modbusreg_2;//2048
extern DEVICE_REG_3 modbusreg_3;//3072

extern OS_MUT mutModbusReg;
extern MODBUSFRAME frame;
extern uint16_t ModbusMasterFrameLength(MODBUSFRAME* pFrame);
extern uint16_t ModbusSlaveFrameLength(MODBUSFRAME* pFrame);
extern uint8_t ModbusMasterCheckCRC(MODBUSFRAME* pFrame);
extern void ModbusMakeMasterCRC(MODBUSFRAME* pFrame);
extern uint8_t ModbusSlaveCheckCRC(MODBUSFRAME* pFrame);
extern void ModbusReadRegs(MODBUSFRAME* pFrame);
extern void ModbusWriteRegs(MODBUSFRAME* pFrame);
extern void ModbusInit(void);
extern void OnAccessTimerExpire(void);
extern uint8_t IsEraseCMD(uint16_t addrbegin, uint16_t addrend);
extern uint8_t IsUpdateBinData(uint16_t addrbegin, uint16_t addrend);
extern uint8_t IsUpdateCMD(uint16_t addrbegin, uint16_t addrend);
extern uint8_t IsRebootCMD(uint16_t addrbegin, uint16_t addrend);
extern void GetModbusBinData(BINDATA* bindata);
extern void ModbusI322Reg(uint16_t* pReg, int32_t nValue);
extern void ModbusI642Reg(uint16_t* pReg, int64_t nValue);
extern void ModbusFloat2Reg(uint16_t* pReg, float fValue);
extern void ModbusDouble2Reg(uint16_t* pReg, double nValue);
extern int32_t ModbusReg2Int(uint16_t* pReg);
extern float ModbusReg2Float(uint16_t* pReg);
extern double ModbusReg2Double(uint16_t* pReg);
extern  void ModbusFloat2Byte(uint8_t* pData, float fValue);
extern uint8_t IsConfigWritten(uint16_t addrbegin, uint16_t addrend);
extern uint8_t IsReadPeakVale(uint16_t addrbegin, uint16_t addrend);
extern uint8_t IsConfigWritten(uint16_t addrbegin, uint16_t addrend);
#endif
