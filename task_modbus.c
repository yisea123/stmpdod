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
//这个进程用于与交互数据
static void ModbusSendFrame(MODBUSFRAME* pFrame)
{
	SendData.pData = (uint8_t*)pFrame;
	SendData.datalen = ModbusSlaveFrameLength(pFrame);
	UART1_Send(SendData.pData, SendData.datalen);
}

__task void ModbusProc(void)
{
	MODBUSFRAME* pFrame;//modbus rtu通信协议
	os_mbx_init(&mailbox_frame, sizeof(mailbox_frame));//一开始阻塞
	while (os_mbx_wait(&mailbox_frame, (void**)&pFrame, 0xFFFF) == OS_R_MBX) { //收到上位机的请求或者是数据（升级等用）
		wd_modbus = 20;
		if (pFrame->header.slaveaddr == config.slaveaddr) {//从机
			if (ModbusMasterCheckCRC(pFrame)) {//crc校验
				uint16_t addrbegin = MAKEWORD(pFrame->master16.addrlo, pFrame->master16.addrhi);
				uint16_t addrend = addrbegin + MAKEWORD(pFrame->master16.numlo, pFrame->master16.numhi);
				switch (pFrame->header.func) {//根据func选择一个功能。
					case 3://这里就是发送数据给上位机，可以根据地址不同发送不同的数据。
						if (addrend <= 64 || addrbegin >= 2048) {
							//如果小小于等于64就会发送版本号
							 if (addrbegin >= 3072 && addrend <= (3072 + 125) && queue_empty(&m_queue)) { //当峰值寄存器没有更新的时候，不返回数据
								continue;
								} else if (addrbegin >= 3072 && addrend <= (3072 + 125) && !queue_empty(&m_queue)) {//多个进程之间通信用邮箱机制（实时系统）
								item_t ppvalue;
								queue_dequeue(&m_queue, &ppvalue);
								memcpy(&modbusreg_3, &ppvalue, sizeof(modbusreg_3)); //取出发送的数据
							
							}
							ModbusReadRegs(pFrame);//处理 填充ppvalue
							os_dly_wait(1);//等待数据填充完毕
							ModbusSendFrame(pFrame);//发送
						}
						break;
					case 16://得到数据写入寄存器，升级stm32代码用
						ModbusWriteRegs(pFrame);
						{
							if (IsEraseCMD(addrbegin, addrend)) {
								wd_modbus = 50;
								EraseBinArea(MAKELONG(modbusreg_1.addr[0], modbusreg_1.addr[1]));//擦除
							}
							if (IsUpdateBinData(addrbegin, addrend)) {//
								BINDATA bin;
								GetModbusBinData(&bin);//填充数据
								UpdateBinData(&bin);//写入flash
							}
							if (IsUpdateCMD(addrbegin, addrend)) {
								LoadUpdate();
							}
							if (IsRebootCMD(addrbegin, addrend)) {
								Reboot();//复位
							}
							if (IsConfigWritten(addrbegin, addrend)) {
								memcpy(&config, &modbusreg_2, sizeof(modbusreg_2));
								SaveConfig2Flash();
								Reboot();
							}
						}
						ModbusSendFrame(pFrame);//发送
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
	isr_mbx_send(&mailbox_frame, ptr);//可以唤醒上面线程。在串口源文件中调用该函数。
}

void StopTaskModbus(void)
{
	os_tsk_delete(tid);
}

