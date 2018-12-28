/*
 * File:   main.c
 * Author: Grupa Autora
 *
 * Created on December 28, 2018, 12:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <p30Fxxxx.h>
#include "uart.h"
#include "driverGLCD.h"
#include "frame_utils.h"
#include "touchscreen.h"

#define FCY 2500000UL
#include <libpic30.h>

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


unsigned int pwd_array[4];
unsigned int pwd_cnt = 0;
unsigned int pwd_rd = 0;

int main(int argc, char** argv) {
  unsigned char jovo[32] = "Jovo Vojvoda\n";
  unsigned int X = 0;
  unsigned int Y = 0;
  int PWD_in = 0;
  int i = 0;

	ConfigureLCDPins();
  initTouchScreen();
	GLCD_LcdInit();
	GLCD_ClrScr();
  initUART1();

	TRISFbits.TRISF6=0;//konfigurisemo kao izlaz

  drawNumpad();

  RS232_putst("\n----------------------");
  RS232_putst("\nEnter Password!\n");
	/* LATFbits.LATF6=~LATFbits.LATF6; */

  while(1){

    PWD_in = getNumTS();
    if(PWD_in != -1 && pwd_rd == 0){
      RS232_putst("\n");
      RS232_putst("pwd_cnt: ");
      WriteUART1dec2string(PWD_in);
      RS232_putst("\n");
      pwd_rd = 1;
      pwd_array[pwd_cnt] = PWD_in;
      drawPwdIndicator(pwd_cnt);
      pwd_cnt++;
    }
    else if(PWD_in == -1)
      pwd_rd = 0;

    if(pwd_cnt == 4){
      RS232_putst("\nPWD_entered: \n");
      for(i = 0; i<4; i++){
        WriteUART1dec2string(pwd_array[i]);
        RS232_putst("\n");
      }
      pwd_cnt = 0;
    }
  }
    return (EXIT_SUCCESS);
}
