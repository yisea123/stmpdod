#include "std.h"
#include "main.h"
#include "task_modbus.h"
#include "driver/uart1.h"
#include "driver/modbus.h"
#include "driver/update.h"
#include "driver/ADC.h"
#include "driver/crc16.h"
#include <stdlib.h>
#include "main.h"
#include "task_time.h"
#include "config.h"
#include "queue.h"
uint32_t wd_modbus = 0xFFFFFFFF;
static uint64_t stk[(600 + sizeof(BINDATA)) / 8];
static OS_TID tid;
static os_mbx_declare(mailbox_frame, 1);
static SENDDATA SendData;
static void ModbusSendFrame(MODBUSFRAME* pFrame)
{
	SendData.pData = (uint8_t*)pFrame;
	SendData.datalen = ModbusSlaveFrameLength(pFrame);
	UART1_Send(SendData.pData, SendData.datalen);
}

__task void ModbusProc(void)
{
	MODBUSFRAME* pFrame;
	os_mbx_init(&mailbox_frame, sizeof(mailbox_frame));
	while (os_mbx_wait(&mailbox_frame, (void**)&pFrame, 0xFFFF) == OS_R_MBX) {
		wd_modbus = 20;
		if (pFrame->header.slaveaddr == config.slaveaddr) {
			if (ModbusMasterCheckCRC(pFrame)) {
				uint16_t addrbegin = MAKEWORD(pFrame->master16.addrlo, pFrame->master16.addrhi);
				uint16_t addrend = addrbegin + MAKEWORD(pFrame->master16.numlo, pFrame->master16.numhi);
				switch (pFrame->header.func) {
					case 3:
						if (addrend <= 64 || addrbegin >= 2048) {
							if (addrbegin >= 3072 && addrend <= (3072 + 125) && queue_empty(&m_queue)) { //当峰值寄存器没有更新的时候，不返回数据
								continue;
							} else if (addrbegin >= 3072 && addrend <= (3072 + 125) && !queue_empty(&m_queue)) {
								item_t ppvalue;
								queue_dequeue(&m_queue, &ppvalue);
								memcpy(&modbusreg_3, &ppvalue, sizeof(modbusreg_3));
							}
							ModbusReadRegs(pFrame);
							os_dly_wait(1);
							ModbusSendFrame(pFrame);
						}
						break;
					case 16:
						ModbusWriteRegs(pFrame);
						{
							if (IsEraseCMD(addrbegin, addrend)) {
								wd_modbus = 50;
								EraseBinArea(MAKELONG(modbusreg_1.addr[0], modbusreg_1.addr[1]));
							}
							if (IsUpdateBinData(addrbegin, addrend)) {
								BINDATA bin;
								GetModbusBinData(&bin);
								UpdateBinData(&bin);
							}
							if (IsUpdateCMD(addrbegin, addrend)) {
								LoadUpdate();
							}
							if (IsRebootCMD(addrbegin, addrend)) {
								Reboot();
							}
							if (IsConfigWritten(addrbegin, addrend)) {
								memcpy(&config, &modbusreg_2, sizeof(modbusreg_2));
								SaveConfig2Flash();
								Reboot();
							}
						}
						ModbusSendFrame(pFrame);
						break;
				}
			}

		}
		pFrame = 0;
		wd_modbus = 0xFFFFFFFF;
	}
}

void CreateTaskModbus(void)
{
	tid = os_tsk_create_user(ModbusProc, PRIORITY_MEDIUM, stk, sizeof(stk));
}

void ISR_UARTRS485_0(uint8_t* ptr)
{
	isr_mbx_send(&mailbox_frame, ptr);
}

void StopTaskModbus(void)
{
	os_tsk_delete(tid);
}

