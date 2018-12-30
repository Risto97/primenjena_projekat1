#ifndef ALC_TEST_C
#define ALC_TEST_C

#include "adc.h"
#include "alc_test.h"
#include "uart.h"

void initAlc(){
  ConfigureADCPins_Alc();
  TMR4_init();
}

#define TMR4_period 48828 //  Fosc = 2.5MHz, FOSC/4, prescaler = 64, 1s => PR2 = 39062
void TMR4_init()
{
	TMR4 = 0;
	PR4 = TMR4_period;

	T4CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
  T4CONbits.TCKPS = 3;
	IFS1bits.T4IF = 0; // clear interrupt flag
	IEC1bits.T4IE = 1; // enable interrupt

	T4CONbits.TON = 0; // T4 on
}
void TMR4_start(){
  if(T4CONbits.TON == 0)
    T4CONbits.TON = 1; // T4 on
}
void TMR4_stop(){
	T4CONbits.TON = 0; // T4 on
}


int getAlcTest(){
  unsigned int res;
  res = getADCbuf0();

  if(res > 1000)
    return -1;

  return 0;
}

#endif
