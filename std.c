#include "std.h"

void Reboot(void) //¸´Î»
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
	WWDG_SetPrescaler(WWDG_Prescaler_8);
	WWDG_SetWindowValue(80);
	WWDG_Enable(127);
	WWDG_SetCounter(127);
}

void delay_us(int cnt)
{
	cnt *= 9;
	while (cnt--);
}
