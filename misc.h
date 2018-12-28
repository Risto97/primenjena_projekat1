#ifndef MISC_H
#define MISC_H

#include<p30Fxxxx.h>

void initServo();
void ConfigureServoPins();
void TMR5_stop();
void TMR5_start();
void TMR5_init();
void TMR5_set(unsigned int pr);
void ServoStop();
void ServoStart(unsigned int direction);
void initBuzzer();
void BuzzerStart();
void BuzzerStop();

#endif
