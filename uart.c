#ifndef UART_C
#define UART_C

#include "uart.h"

char tempRX;
unsigned int wordReceived = 0;
char buff[32];
unsigned int len = 0;
int n = 0;
void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void) {
  IFS0bits.U1RXIF = 0;
  tempRX = U1RXREG;
  buff[n] = tempRX;
  n++;
  if(buff[n-1] == 13){  // 13 is ascii for carriage return
    buff[n-1] = '\0';
    wordReceived = 1;
    len = n;
    n = 0;
  }
}

int getBuff(){
  unsigned int len_tmp = len;
  if(wordReceived == 1){
    len = 0;
    wordReceived = 0;
    return len_tmp;
  }
  else
    return -1;
}
char *rbuff(){
  return buff;
}

void initUART1()
{
  U1BRG=0x0015;//ovim odredjujemo baudrate
  U1MODEbits.ALTIO=0;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne
  IEC0bits.U1RXIE=1;//omogucavamo rx1 interupt
  U1STA&=0xfffc;
  U1MODEbits.UARTEN=1;//ukljucujemo ovaj modul
  U1STAbits.UTXEN=1;//ukljucujemo predaju
  TRISFbits.TRISF2 = 1;
  TRISFbits.TRISF3 = 0;
}

void RS232_putst(char *str){
  while((*str)!=0) {
    WriteUART1(*str);
    if (*str==13) WriteUART1(10);
    if (*str==10) WriteUART1(13);
    str++;
  }
}

void WriteUART1(unsigned int data){

  while(!U1STAbits.TRMT);

  if(U1MODEbits.PDSEL == 3)
    U1TXREG = data;
  else
    U1TXREG = data & 0xFF;
}

/***********************************************************************
 * Ime funkcije      : WriteUART1dec2string                     		   *
 * Opis              : Funkcija salje 4-cifrene brojeve (cifru po cifru)*
 * Parameteri        : unsigned int data-podatak koji zelimo poslati    *
 * Povratna vrednost : Nema                                             *
 ************************************************************************/
void WriteUART1dec2string(int data)
{
	unsigned char temp;
  if(data < 0){
    WriteUART1('-');
    data = -data;
  }

	temp=data/1000;
  if(temp != 0)
    WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
  if(temp != 0)
    WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
  if(temp != 0)
    WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
}


#endif
