#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#define TMR2_period 39062/32 //  Fosc = 2.5MHz, FOSC/4, prescaler = 64, 1s => PR2 = 39062

#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14

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
