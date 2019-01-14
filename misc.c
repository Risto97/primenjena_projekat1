#ifndef MISC_C
#define MISC_C

#include "misc.h"

unsigned int servo_tmr_vals[2][2] = {{48550, 1450},
                                     {44500, 5500}};
unsigned int servo_dir = 0;
unsigned int servo_phase_cnt = 0;
unsigned int buzz_servo = 1;
void __attribute__ ((__interrupt__, no_auto_psv)) _T5Interrupt(void)
{
  TMR5 = 0;
  TMR5_stop();

  if(buzz_servo == 0){
  LATAbits.LATA11=~LATAbits.LATA11;
  }
  else{
    TMR5_set(servo_tmr_vals[servo_dir][servo_phase_cnt]);
    switch(servo_phase_cnt){
    case 0:
      LATFbits.LATF6=0;
      servo_phase_cnt = 1;
      break;
    case 1:
      LATFbits.LATF6=1;
      servo_phase_cnt = 0;
      break;
    }
  }
  TMR5_start();

	IFS1bits.T5IF = 0;
}

#define TMR5_period 39062 //  Fosc = 2.5MHz, FOSC/4, prescaler = 64, 1s => PR2 = 39062
void TMR5_init()
{
	TMR5 = 0;
	PR5 = TMR5_period;

	T5CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
  T5CONbits.TCKPS = 0;
	IFS1bits.T5IF = 0; // clear interrupt flag
	IEC1bits.T5IE = 1; // enable interrupt

	T5CONbits.TON = 0; // T5 on
}

void TMR5_set(unsigned int pr){
	PR5 = pr;
}
void TMR5_start(){
  if(T5CONbits.TON == 0)
    T5CONbits.TON = 1; // T5 on
}
void TMR5_stop(){
	T5CONbits.TON = 0; // T5 on
}

void ServoStart(unsigned int direction){
  buzz_servo = 1;
  servo_dir = direction;
  TMR5_set(servo_tmr_vals[direction][1]);
	LATFbits.LATF6=1;
  TMR5_start();
}
void ServoStop(){
  TMR5_stop();
	LATFbits.LATF6=0;
}
void initServo(){
  buzz_servo = 1;
  ConfigureServoPins();
  TMR5_init();
}

void ConfigureServoPins(){
	TRISFbits.TRISF6=0;
	LATFbits.LATF6=0;

}

void BuzzerStart(unsigned int pr){
  buzz_servo = 0;
  TMR5_set(pr);
  TMR5_start();
}
void BuzzerStop(){
  TMR5_stop();
  LATAbits.LATA11 = 0;
}

void initBuzzer(){
  buzz_servo = 0;
  TRISAbits.TRISA11=0;
  TMR5_init();
}

void initPIR(){
  TRISDbits.TRISD8=1;
}

int readPIR(){
  return PORTDbits.RD8;
}


#endif
