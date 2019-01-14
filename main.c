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
unsigned int pwd[PWD_LEN];
unsigned int pwd_entry[PWD_LEN];
/*************************/

/* commands */
#define INVALID -1
#define OPEN 0
#define CLOSE 1
#define ADDPWD 2
#define RMPWD 3
#define HELP 4
char *open_cmd = "open";
char *close_cmd = "close";
char *addpwd_cmd = "addpwd";
char *rmpwd_cmd = "rmpwd";
char *help_cmd = "help";
#define prompt "\n[sudo]$ "
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
int rmPwdEeprom();
void noPwdMessages();
/*************************************/

unsigned int TMR4_soft_cnt = 0;
unsigned int alc_test_timeout = 0;
unsigned int opened_door_timeout = 0;
void __attribute__((__interrupt__, no_auto_psv )) _T4Interrupt(void)
{
  TMR4 =0;
  TMR4_soft_cnt++;
  if(TMR4_soft_cnt == 2){
    TMR4_soft_cnt = 0;
    alc_test_timeout = 1;
    opened_door_timeout = 1;
    TMR4_stop();
  }

	IFS1bits.T4IF = 0;
}


int main(int argc, char** argv) {
  int PWD_in = 0;
  int no_alc = 1;
  int cmd = -1;
  int sudo = 0;
  int pwd_cnt = 0;
  int pwd_rx_cnt = 0;
  int pwd_rd = 0;
  int pwd_saved_num;
  int receiving_pwd = 0;

  char *rbuff_rx;

	ConfigureLCDPins();
  initTouchScreen();
  initServo();
  initPIR();
  initBuzzer();
  initAlc();
	ADCinit_TS();
  ADCstart();
	GLCD_LcdInit();
	GLCD_ClrScr();
  initUART1();

  /* check if passwords are saved */
  pwd_saved_num = checkEepromSavedPWDs();
  if(checkEepromInitialized(init_message_eeprom) == -1 || pwd_saved_num <= 0){
    pwd_saved_num = 0;
    initEeprom(init_message_eeprom);
    noPwdMessages();
  }
  else if( pwd_saved_num > 0 && checkEepromInitialized(init_message_eeprom)){
    WriteUART1dec2string(pwd_saved_num);
    RS232_putst(" users are registered\n");
    drawNumpad();
  }

  RS232_putst(prompt);
  CloseDoors();
  while(1){
    /* Check if command is received over UART
       set flags for given command
          *sudo indicates that doors are controlled by UART
    */
    if(getBuff() > 0){
      rbuff_rx = rbuff();
      if(receiving_pwd == 0){  // receiving command from UART
        cmd = decodeCMD(rbuff_rx);

        if(cmd == OPEN){
          pwd_rd = 0;
          pwd_cnt = 0;
          sudo = 1;
          RS232_putst("\nExecuting open\n");
          drawSudoOpen();
          OpenDoors();
        }
        if(cmd == CLOSE){
          sudo = 0;
          RS232_putst("\nExecuting close\n");
          CloseDoors();
          if(pwd_saved_num > 0)
            drawNumpad();
          else
            drawNoPwdEeprom();
        }
        if(cmd == ADDPWD){
          RS232_putst("\nExecuting addpwd");
          receiving_pwd = 1;
        }
        if(cmd == RMPWD){
          sudo = 1;
          RS232_putst("\nExecuting rmpwd\n");
          pwd_saved_num = rmPwdEeprom();
          noPwdMessages();
        }
        if(cmd == HELP){
          RS232_putst("\nCommands available\n");
          RS232_putst("<open> Open doors as sudo\n");
          RS232_putst("<close> Close doors as sudo\n");
          RS232_putst("<addpwd> Add new password to EEPROM,\n         type 4 numbers 0-9 followed by enter after each number\n");
          RS232_putst("<rmpwd> Remove all saved passwords\n");
        }
        RS232_putst(prompt);
      }
      else{
        WriteUART1((unsigned int)'*');
        pwd[pwd_rx_cnt] = (rbuff_rx[0]-'0');
        pwd_rx_cnt++;
        if(pwd_rx_cnt == PWD_LEN){
          pwd_rx_cnt = 0;
          pwd_saved_num++;
          writePwdEeprom(pwd);
          receiving_pwd = 0;
          drawNumpad();
          RS232_putst("\nPassword added\n");
          RS232_putst(prompt);
          sudo = 0;
        }
      }
    }
    /* Else doors are controlled by MCU */
    else if(sudo == 0 && pwd_saved_num != 0){

      /* Reading input from touchscreen */
      PWD_in = getNumTS();
      if(PWD_in != -1 && pwd_rd == 0){
        pwd_rd = 1;
        pwd_entry[pwd_cnt] = PWD_in;
        drawPwdIndicator(pwd_cnt);
        pwd_cnt++;
        Buzz(70, 4000);
      }
      else if(PWD_in == -1)
        pwd_rd = 0;
      if(pwd_cnt == PWD_LEN){  // Entered all numbers from touchscreen
        pwd_cnt = 0;

        if(check_password(pwd_entry, PWD_LEN) != -1){ //PWD correct
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
            opened_door_timeout = 0;
            TMR4_start();
            while(!readPIR() && !opened_door_timeout);
            opened_door_timeout = 0;
            __delay_ms(1000);
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

void noPwdMessages(){
  RS232_putst("No users are registered\n");
  RS232_putst("Please add password\n");
  RS232_putst("Type <help> command for more info\n");
  drawNoPwdEeprom();

}
int rmPwdEeprom(){
  Eeprom_WriteWord(PWD_NUM_ADDR, 0);
  return 0;
}
void writePwdEeprom(unsigned int pwd_in[PWD_LEN]){
  unsigned int i, wr_addr;
  unsigned int pwd_num = Eeprom_ReadWord(PWD_NUM_ADDR);
  pwd_num++;
  Eeprom_WriteWord(PWD_NUM_ADDR, pwd_num);
  for(i = 0; i<PWD_LEN; i++){
    wr_addr = PWD_BASE_ADDR + (pwd_num-1)*PWD_LEN + i;
    Eeprom_WriteWord(wr_addr, pwd_in[i]);
  }
}
void readPwdEeprom(unsigned int *pwd_pt, int pwd_num){
  unsigned int base_addr = PWD_BASE_ADDR + pwd_num*PWD_LEN;
  int i = 0;
  for(i=0; i<PWD_LEN; i++){
    pwd_pt[i] = Eeprom_ReadWord(base_addr+i);
  }
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

  if(pwd_num == 0)
    return -1;
  for(j=0; j<pwd_num; j++){
    readPwdEeprom(pwd, j);
    if(cmpArray(pwd, entry, PWD_LEN) == 1)
      return j;
  }
  return -1;
}

int cmpArray(unsigned int *ar1, unsigned int *ar2, unsigned int len){
  int i;
  for(i=0; i<len; i++){
    if(ar1[i] != ar2[i]){
      return -1;
    }
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
    RS232_putst("\nCommand open received!");
    return OPEN;
  }
  else if(strcmp(close_cmd, cmd) != -1){
    RS232_putst("\nCommand close received!");
    return CLOSE;
  }
  else if(strcmp(addpwd_cmd, cmd) != -1){
    RS232_putst("\nCommand addpwd received!");
    return ADDPWD;
  }
  else if(strcmp(rmpwd_cmd, cmd) != -1){
    RS232_putst("\nCommand rmpwd received!");
    return RMPWD;
  }
  else if(strcmp(help_cmd, cmd) != -1){
    RS232_putst("\nCommand help received!\n");
    return HELP;
  }


  RS232_putst("\nInvalid command!\n");
  RS232_putst(cmd);
  RS232_putst("\n");
  return INVALID;
}
