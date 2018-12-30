#ifndef UART_H
#define UART_H

#include <p30Fxxxx.h>

void initUART1();
void RS232_putst(char *str);
void WriteUART1(unsigned int data);
void WriteUART1dec2string(int data);
int getBuff();
char *rbuff();

#endif
