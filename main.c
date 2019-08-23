#include "std.h"
#include "main.h"
#include "driver/rcc.h"
#include "driver/gpio.h"
#include "driver/nvic.h"
#include "driver/modbus.h"
#include "driver/ADC.h"
#include "driver/uart1.h"
#include "task_modbus.h"
#include "task_time.h"
#include "driver/ADC.h"
#include <stdlib.h>

static U64 stk[600 / 8];//栈大小

static __task void init(void)
{
	Config_Init();
	UART1_Init();
	ModbusInit();
	CreateTaskModbus();
	CreatTaskTime();
	InitADC();//开始模数转换，采集到的样本自动传入内存中Apoint数组，触发处理。
	os_tsk_prio_self(PRIORITY_MEDIUM);
	while (1) {
		os_tsk_pass();
	}
}

int main(void)
{
	RCC_Configuration(); // 配置
	GPIO_Configuration();
	NVIC_Configuration();
	os_sys_init_user(init, PRIORITY_MEDIUM, stk, sizeof(stk));
}

void HardFault_Handler(void)
{
	Reboot();
}
