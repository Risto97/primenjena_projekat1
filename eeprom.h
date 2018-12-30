#ifndef EEPROM_H
#define EEPROM_H

#include <p30Fxxxx.h>

#define ERASE_WORD 0x4044
#define WRITE_WORD 0x4004
#define ADDRESS_HI 0x007F
#define EEPROM_LOW_START_ADDRESS 0xFC00
#define TRUE 1


void Eeprom_WriteWord(unsigned short pushAddressOffset, unsigned short value);
unsigned short Eeprom_ReadWord(unsigned short pushAddressOffset);

#endif
