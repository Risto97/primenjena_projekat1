#ifndef FRAME_UTILS_C
#define FRAME_UTILS_C

#include "frame_utils.h"

unsigned int bar_start = (128-(4*BAR_WIDTH+3*BAR_SPACE))/2;
int rect_width = (int)128/5;
int rect_height = (int)64/3+3;

void drawPwdIndicator(int num){
  GLCD_Circle(bar_start+num*BAR_WIDTH+num*BAR_SPACE+(BAR_WIDTH/2), 58, 3);
}

void drawNumpad(){
  GLCD_ClrScr();
  drawNumpad_noClr();
}

void drawNumpad_noClr(){
  int x;
  int y;
  int i;

  for(y=0; y<NUMPAD_ROW_NUM-1; y++){
    for(x=0; x<NUMPAD_COL_NUM; x++){
      GLCD_Rectangle(rect_width*x,
                     rect_height*y,
                     rect_width*(x+1),
                     rect_height*(y+1));

      GoToXY(10+x*rect_width, 1+y*3);
      Glcd_PutCharBig(48+i);
      i++;
    }
  }

  for(i=0; i<4; i++){
    GLCD_Rectangle(bar_start+i*BAR_WIDTH+i*BAR_SPACE, 63, bar_start+(i+1)*BAR_WIDTH+i*BAR_SPACE, 63);
  }
}

void printf_Big(char *str){
  int i = 0;
  while(str[i] != '\0'){
    Glcd_PutCharBig((char)str[i]);
    i++;
  }
}
void drawNoPwdEeprom(){
  GLCD_ClrScr();
  GoToXY(5, 2);
  printf_Big("No passwords");
  GoToXY(5, 4);
  printf_Big("Connect UART!");
}
void drawPasswordCorrect(){
  GLCD_ClrScr();
  GoToXY(32, 2);
  printf_Big("Correct");
  GoToXY(27, 4);
  printf_Big("Password!");
}

void drawPasswordWrong(){
  GLCD_ClrScr();
  GoToXY(32, 2);
  printf_Big("Wrong");
  GoToXY(27, 4);
  printf_Big("Password!");

}

void drawAlcTestInfo(){
  GLCD_ClrScr();
  GoToXY(20, 2);
  printf_Big("Please blow");
  GoToXY(20, 4);
  printf_Big("onto sensor!");
}

void drawAlcTestFail(){
  GLCD_ClrScr();
  GoToXY(0, 2);
  printf_Big("Alcohol detected");
  GoToXY(15, 4);
  printf_Big("Access denied!");
}
void drawAlcTestPass(){
  GLCD_ClrScr();
  GoToXY(0, 3);
  printf_Big("Access Granted!");
}

void drawSudoOpen(){
  GLCD_ClrScr();
  GoToXY(15, 2);
  printf_Big("Sudo mode");
  GoToXY(15, 4);
  printf_Big("Door Open!");
}

#endif
