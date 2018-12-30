/*
 * File:   main.c
 * Author: Grupa Autora
 *
 * Created on December 28, 2018, 12:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include <p30Fxxxx.h>
#include "uart.h"
#include "driverGLCD.h"
#include "frame_utils.h"
#include "touchscreen.h"
#include "adc.h"
#include "alc_test.h"
#include "misc.h"
#include "eeprom.h"

#define FCY 2500000UL
#include <libpic30.h>

/* EEPROM */
#define PWD_NUM_ADDR 128
#define PWD_BASE_ADDR 132
char *init_message_eeprom = "EEPROM initialized\0";
/* PASSWORD */
#define PWD_LEN 4
unsigned int pwd[PWD_LEN]; //= {1,3,5,8};
unsigned int pwd_asd[PWD_LEN] = {6,7,8,9};
unsigned int pwd_array[PWD_LEN];
/*************************/

/* commands */
#define INVALID -1
#define OPEN 0
#define CLOSE 1
#define ADDPWD 2
char *open_cmd = "open";
char *close_cmd = "close";
char *addpwd_cmd = "addpwd";
/*************************/

/* Top level functions */
int check_password(unsigned int *entry,
                   unsigned int len);
void CloseDoors();
void OpenDoors();
void Buzz(unsigned int time, unsigned int pr);
int strcmp(char *str1, char *str2);
int decodeCMD(char *cmd);
int checkEepromInitialized();
void writeStrEeprom(char *str, int addr);
void initEeprom(char *init_message);
int checkEepromSavedPWDs();
void writePwdEeprom(unsigned int pwd_in[PWD_LEN]);
void readPwdEeprom(unsigned int *pwd_pt, int pwd_num);
int cmpArray(unsigned int *ar1, unsigned int *ar2, unsigned int len);
/*************************************/

unsigned int TMR4_soft_cnt = 0;
unsigned int alc_test_timeout = 0;
void __attribute__((__interrupt__, no_auto_psv )) _T4Interrupt(void)
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


int main(int argc, char** argv) {
  int PWD_in = 0;
  int no_alc = 1;
  int cmd = -1;
  int supervisor = 0;
  int pwd_cnt = 0;
  int pwd_rx_cnt = 0;
  int pwd_rd = 0;
  int pwd_saved_num;
  int receiving_pwd = 0;

  char *rbuff_rx;

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

  /* check if passwords are saved */
  pwd_saved_num = checkEepromSavedPWDs();
  if(checkEepromInitialized(init_message_eeprom) == -1 || pwd_saved_num == 0){
    pwd_saved_num = 0;
    initEeprom(init_message_eeprom);
    RS232_putst("No users were registered\n");
    RS232_putst("Please add password\n");
    RS232_putst("With command mkpwd followed by sequence of 4 numbers 0-9\n");
    drawNoPwdEeprom();
  }
  else{
    WriteUART1dec2string(pwd_saved_num);
    RS232_putst(" users were registered\n");
    drawNumpad();
  }

  RS232_putst("Type for supervisor mode\n");
  CloseDoors();
  while(1){
    /* Check if command is received over UART
       set flags for given command
          *supervisor indicates that doors are controlled by UART
    */
    if(getBuff() > 0){
      rbuff_rx = rbuff();
      if(receiving_pwd == 0){
        cmd = decodeCMD(rbuff_rx);

        if(cmd == OPEN){
          pwd_rd = 0;
          pwd_cnt = 0;
          supervisor = 1;
          drawSupervisorOpen();
          OpenDoors();
        }
        if(cmd == CLOSE){
          supervisor = 0;
          CloseDoors();
          if(pwd_saved_num > 0)
            drawNumpad();
          else
            drawNoPwdEeprom();
        }
        if(cmd == ADDPWD){
          RS232_putst("\nExecuting addpwd\n");
          receiving_pwd = 1;
        }
      }
      else{
        WriteUART1(rbuff_rx[0]);
        pwd[pwd_rx_cnt] = (rbuff_rx[0]-'0');
        pwd_rx_cnt++;
        if(pwd_rx_cnt == PWD_LEN){
          pwd_saved_num++;
          writePwdEeprom(pwd);
          receiving_pwd = 0;
        }
      }
    }
    /* Else doors are controlled by MCU */
    else if(supervisor == 0 && pwd_saved_num != 0){

      /* Reading input from touchscreen */
      PWD_in = getNumTS();
      if(PWD_in != -1 && pwd_rd == 0){
        pwd_rd = 1;
        pwd_array[pwd_cnt] = PWD_in;
        drawPwdIndicator(pwd_cnt);
        pwd_cnt++;
        Buzz(70, 4000);
      }
      else if(PWD_in == -1)
        pwd_rd = 0;
      if(pwd_cnt == PWD_LEN){  // Entered all numbers from touchscreen
        pwd_cnt = 0;

        if(check_password(pwd_array, PWD_LEN) != -1){ //PWD correct
          drawPasswordCorrect();
          __delay_ms(2000);
          drawAlcTestInfo(); // begin alcotest
          ADCinit_Alc();
          alc_test_timeout = 0;
          TMR4_start();
          while(no_alc && !alc_test_timeout){ // read alcotest until detected or timed out
            if(getAlcTest() == -1){
              no_alc = 0;
              pwd_cnt = 0;
            }
          }
          alc_test_timeout = 0;
          if(no_alc == 1){ // no alcohol detected, open doors
            no_alc = 1;
            drawAlcTestPass();
            OpenDoors();
            __delay_ms(3000);
            CloseDoors();
          }
          else{ // alcohol is detected
            drawAlcTestFail();
            __delay_ms(2000);
          }
          /* reset flags, and prepare for password entry */
          no_alc = 1;
          drawNumpad();
          ADCinit_TS();
          __delay_ms(500);
        }
        else{  // PWD false
          pwd_cnt = 0;
          drawPasswordWrong();
          __delay_ms(2000);
          drawNumpad();
        }
      }
    }
  }
    return (EXIT_SUCCESS);
}

void writePwdEeprom(unsigned int pwd_in[PWD_LEN]){
  unsigned int i, wr_addr;
  unsigned int pwd_num = Eeprom_ReadWord(PWD_NUM_ADDR);
  pwd_num++;
  Eeprom_WriteWord(PWD_NUM_ADDR, pwd_num);
  for(i = 0; i<PWD_LEN; i++){
    wr_addr = PWD_BASE_ADDR + (pwd_num-1)*PWD_LEN + i;
    Eeprom_WriteWord(wr_addr, pwd_in[i]);
    WriteUART1dec2string(pwd_in[i]);
  }
}
void readPwdEeprom(unsigned int *pwd_pt, int pwd_num){
  unsigned int base_addr = PWD_BASE_ADDR + pwd_num*PWD_LEN;
  int i = 0;
  RS232_putst("Reading password from eeprom: \n");
  for(i=0; i<PWD_LEN; i++){
    pwd_pt[i] = Eeprom_ReadWord(base_addr+i);
    WriteUART1dec2string(pwd_pt[i]);
  }
  RS232_putst("\nEnd reading password from eeprom: \n");
}
void writeStrEeprom(char *str, int addr){
  int i = 0;
  while(str[i] != '\0'){
    Eeprom_WriteWord(addr+i, (unsigned short)str[i]);
    i++;
  }
  Eeprom_WriteWord(addr+i, (unsigned short)'\0');
}
int checkEepromInitialized(char *init_message){
  char init_message_rd[32];
  int i = 0;

  while(i < 32){
    init_message_rd[i] = Eeprom_ReadWord(i);
    i++;
  }
  RS232_putst("Reading from eeprom: \n");
  RS232_putst(init_message_rd);
  RS232_putst("\n");

  return strcmp(init_message_rd, init_message);
}
int checkEepromSavedPWDs(){
  return Eeprom_ReadWord(PWD_NUM_ADDR);
}
void initEeprom(char *init_message){
  writeStrEeprom(init_message, 0);
  Eeprom_WriteWord(PWD_NUM_ADDR, 0);
}
int check_password(unsigned int *entry,
                   unsigned int len){
  unsigned int pwd_num = Eeprom_ReadWord(PWD_NUM_ADDR);
  unsigned int pwd[PWD_LEN];
  int j;

  for(j=0; j<pwd_num; j++){
    readPwdEeprom(pwd, j);
    if(cmpArray(pwd, entry, PWD_LEN) == 1)
      return j;
  }
  return -1;
}

int cmpArray(unsigned int *ar1, unsigned int *ar2, unsigned int len){
  int i;
  RS232_putst("\nComparing strings!\n");
  for(i=0; i<len; i++){
    WriteUART1dec2string(ar2[i]);
    WriteUART1dec2string(ar1[i]);
    if(ar1[i] != ar2[i]){
      return -1;
    }
  }
  RS232_putst("\nMatched strings!\n");
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
void Buzz(unsigned int time, unsigned int pr){
  BuzzerStart(pr);
  __delay_ms(time);
  BuzzerStop();
}

int strcmp(char *str1, char *str2){
  int i = 0;
  while(str1[i] == str2[i] && str1[i] != '\0'){
    i++;
  }
  if(str1[i] != str2[i])
    return -1;
  else return i;
}

int decodeCMD(char *cmd){
  if(strcmp(open_cmd, cmd) != -1){
    RS232_putst("\nCommand open received!\n");
    return OPEN;
  }
  else if(strcmp(close_cmd, cmd) != -1){
    RS232_putst("\nCommand close received!\n");
    return CLOSE;
  }
  else if(strcmp(addpwd_cmd, cmd) != -1){
    RS232_putst("\nCommand addpwd received!\n");
    return ADDPWD;
  }

  RS232_putst("\nInvalid command!\n");
  RS232_putst(cmd);
  RS232_putst("\n");
  return INVALID;
}
