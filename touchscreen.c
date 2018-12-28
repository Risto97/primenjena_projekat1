#ifndef TOUCHSCREEN_C
#define TOUCHSCREEN_C

/* #include<p30Fxxxx.h> */
#include "touchscreen.h"
#include "adc.h"
#include "frame_utils.h"
#include "uart.h"

#define TMR2_period 39062/32 //  Fosc = 2.5MHz, FOSC/4, prescaler = 64, 1s => PR2 = 39062

#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14

unsigned int x_val = 0;
unsigned int y_val = 0;
unsigned int X = 0;
unsigned int Y = 0;
unsigned int TS_drivers_cnt = 0;

unsigned int tmr2_intr_flag = 0;

void __attribute__ ((__interrupt__)) _T2Interrupt(void)
{
  TMR2 =0;

  if(TS_drivers_cnt == 2){
    y_val = getADCbuf1();
    TS_drivers_cnt = 0;
  }
  if(TS_drivers_cnt == 0){
    // vode horizontalni tranzistori
    DRIVE_A = 1;
    DRIVE_B = 0;
    LATCbits.LATC13=1;
    LATCbits.LATC14=0;
    TS_drivers_cnt = 1;
  }
  else if(TS_drivers_cnt == 1){
    x_val = getADCbuf0();
    // vode vertikalni tranzistori
    LATCbits.LATC13=0;
    LATCbits.LATC14=1;
    DRIVE_A = 0;
    DRIVE_B = 1;
    TS_drivers_cnt = 2;
  }

	IFS0bits.T2IF = 0;

}

void TMR2_init(void)
{
	TMR2 = 0;
	PR2 = TMR2_period;

	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
  T2CONbits.TCKPS = 2;
	IFS0bits.T2IF = 0; // clear interrupt flag
	IEC0bits.T2IE = 1; // enable interrupt

	T2CONbits.TON = 0; // T2 on
}

void TMR2_start(){
  if(T2CONbits.TON == 0)
    T2CONbits.TON = 1; // T2 on
}
void TMR2_stop(){
	T2CONbits.TON = 0; // T2 on
}

int getNumTS(){
  unsigned int x_read = getX();
  unsigned int y_read = getY();
  int rect_width = (int)128/5;
  int rect_height = (int)64/3+3;
  if( y_read < 64 && y_read > 64-rect_height){
    if(x_read > 0 && x_read < rect_width)
      return 0;
    else if( x_read >= rect_width && x_read < 2*rect_width)
      return 1;
    else if( x_read >= 2*rect_width && x_read < 3*rect_width)
      return 2;
    else if( x_read >= 3*rect_width && x_read < 4*rect_width)
      return 3;
    else if( x_read >= 4*rect_width && x_read < 5*rect_width)
      return 4;
  }
  else if(y_read <= 64-rect_height && y_read > 64-2*rect_height){
    if(x_read > 0 && x_read < rect_width)
      return 5;
    else if( x_read >= rect_width && x_read < 2*rect_width)
      return 6;
    else if( x_read >= 2*rect_width && x_read < 3*rect_width)
      return 7;
    else if( x_read >= 3*rect_width && x_read < 4*rect_width)
      return 8;
    else if( x_read >= 4*rect_width && x_read < 5*rect_width)
      return 9;
  }
  return -1;
}


unsigned int getX(){
  X=(x_val-161)*0.03629;
  return X;
}
unsigned int getY(){
  Y=((y_val-500)*0.020725);
  return Y;
}

void initTouchScreen(){
  ConfigureTSPins();
	ConfigureADCPins_TS();

  TMR2_init();
  TMR2_start();
}

void ConfigureTSPins(void)
{
	TRISCbits.TRISC13=0;
  TRISCbits.TRISC14=0;
}

#endif
