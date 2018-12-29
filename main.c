/*
 * File:   main.c
 * Author: Grupa Autora
 *
 * Created on December 28, 2018, 12:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <p30Fxxxx.h> */
#include "uart.h"
#include "driverGLCD.h"
#include "frame_utils.h"
#include "touchscreen.h"
#include "adc.h"
#include "alc_test.h"
#include "misc.h"

#define FCY 2500000UL
#include <libpic30.h>

/* _FOSC(CSW_ON_FSCM_OFF & HS3_PLL4); */
/* _FWDT(WDT_OFF); */
/* _FGS(CODE_PROT_OFF); */

#pragma config FOSFPR = HS3_PLL4        // Oscillator (HS3 w/PLL 4x)
#pragma config FCKSMEN = CSW_ON_FSCM_OFF// Clock Switching and Monitor (Sw Enabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

unsigned int TMR4_soft_cnt = 0;
unsigned int alc_test_timeout = 0;
void __attribute__ ((__interrupt__)) _T4Interrupt(void)
{
  TMR4 =0;
  TMR4_soft_cnt++;
  if(TMR4_soft_cnt == 1){
    TMR4_soft_cnt = 0;
    alc_test_timeout = 1;
    TMR4_stop();
  }

	IFS1bits.T4IF = 0;
}

int check_password(unsigned int *pwd,
                    unsigned int *entry,
                    unsigned int len){
  int i = 0;
  for(i=0; i<len; i++){
    if(pwd[i] != entry[i])
      return -1;
  }
  return 1;
}

void CloseDoors(){
  ServoStart(1);
  __delay_ms(1000);
  ServoStop();
}
void OpenDoors(){
  ServoStart(0);
  __delay_ms(1000);
  ServoStop();
}

unsigned int pwd[4] = {1,3,5,8};
unsigned int pwd_array[4];
unsigned int pwd_cnt = 0;
unsigned int pwd_rd = 0;

const unsigned char *open_cmd = "open";
const unsigned char *close_cmd = "close";

int strcmp(unsigned char *str1, unsigned char *str2){
  int i = 0;
  while(str1[i] == str2[i] && str1[i] != '\0'){
    i++;
  }
  if(str1[i] != str2[i])
    return -1;
  else return i;
}

int decodeCMD(unsigned char *cmd){
  if(strcmp(open_cmd, cmd) != -1){
    RS232_putst("\nCommand open received!\n");
    return 0;
  }
  else if(strcmp(close_cmd, cmd) != -1){
    RS232_putst("\nCommand close received!\n");
    return 1;
  }

  RS232_putst("\nInvalid command!\n");
  return -1;
}

int main(int argc, char** argv) {
  unsigned int X = 0;
  unsigned int Y = 0;
  int PWD_in = 0;
  int i = 0;
  int no_alc = 1;
  int cmd = -1;
  int supervisor = 0;

  unsigned char *rbuff2;

	ConfigureLCDPins();
  initTouchScreen();
  initServo();
  initBuzzer();
  initAlc();
	ADCinit_TS();
  ADCstart();
	GLCD_LcdInit();
	GLCD_ClrScr();
  initUART1();

  drawNumpad();

  RS232_putst("Type for supervisor mode\n");
  CloseDoors();
  while(1){
    if(getBuff() > 0){
      rbuff2 = rbuff();
      RS232_putst("\n");
      RS232_putst(rbuff2);
      cmd = decodeCMD(rbuff2);
      if(cmd == 0){
        pwd_rd = 0;
        pwd_cnt = 0;
        supervisor = 1;
        GLCD_ClrScr();
        drawSupervisorOpen();
        OpenDoors();
      }
      if(cmd == 1){
        supervisor = 0;
        GLCD_ClrScr();
        CloseDoors();
        drawNumpad();
      }
    }

    else if(supervisor == 0){
      PWD_in = getNumTS();
      if(PWD_in != -1 && pwd_rd == 0){
        pwd_rd = 1;
        pwd_array[pwd_cnt] = PWD_in;
        drawPwdIndicator(pwd_cnt);
        pwd_cnt++;
        BuzzerStart(700+PWD_in*20);
        __delay_ms(100);
        BuzzerStop();
      }
      else if(PWD_in == -1)
        pwd_rd = 0;

      if(pwd_cnt == 4){
        pwd_cnt = 0;

        GLCD_ClrScr();
        if(check_password(pwd, pwd_array, 4) == 1){
          drawPasswordCorrect();
          __delay_ms(2000);
          GLCD_ClrScr();
          drawAlcTestInfo();
          ADCinit_Alc();
          TMR4_start();
          while(no_alc && !alc_test_timeout){
            if(getAlcTest() == -1){
              no_alc = 0;
              pwd_cnt = 0;
              pwd_rd = 0;
              GLCD_ClrScr();
              drawAlcTestFail();
              __delay_ms(2000);
            }
          }
          alc_test_timeout = 0;
          if(no_alc == 1){
            no_alc = 1;
            GLCD_ClrScr();
            drawAlcTestPass();
            TMR4_start();
            OpenDoors();
            while(!alc_test_timeout);
            alc_test_timeout = 0;
            CloseDoors();
          }
          no_alc = 1;
          GLCD_ClrScr();
          drawNumpad();
          ADCinit_TS();
          __delay_ms(500);
        }
        else{
          pwd_cnt = 0;
          pwd_rd = 0;
          drawPasswordWrong();
          __delay_ms(2000);
          GLCD_ClrScr();
          drawNumpad();
        }
      }
    }
  }
    return (EXIT_SUCCESS);
}
