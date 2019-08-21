#pragma once

extern unsigned short CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern unsigned short CRC16_Push(unsigned short crc, unsigned char* puchMsg, unsigned short usDataLen);
