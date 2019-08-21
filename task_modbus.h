#ifndef __TASK_MODBUSUART_H__
#define __TASK_MODBUSUART_H__
extern uint32_t wd_modbus;

extern void CreateTaskModbus(void);
extern void NotifyModbusFrame(uint8_t* ptr);
extern void StopTaskModbus(void);
extern void SetGain(uint16_t gain);
extern void Config_Init(void);
#endif
