#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

/* #define FCY 25000000UL */
#define FCY 2500000UL
#include <libpic30.h>

unsigned int getX();
unsigned int getY();


void initTouchScreen();
void ConfigureTSPins(void);
void initADC_TS();
int getNumTS();


void TMR2_start();
void TMR2_stop();
void TMR2_init(void);

#endif
