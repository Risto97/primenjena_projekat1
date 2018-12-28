#ifndef ADC_H
#define ADC_H

#include<p30Fxxxx.h>

void ConfigureADCPins_TS(void);
void ConfigureADCPins_Alc();
void ADCinit_TS(void);
void ADCinit_Alc(void);
unsigned int getADCbuf0();
unsigned int getADCbuf1();
void ADCstart();
void ADCstop();

#endif
