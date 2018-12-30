#ifndef FRAME_UTILS_H
#define FRAME_UTILS_H

#include "driverGLCD.h"

#define NUMPAD_ROW_NUM 3
#define NUMPAD_COL_NUM 5

#define BAR_WIDTH 15
#define BAR_SPACE 4

void drawNumpad();
void drawNumpad_noClr();
void drawPwdIndicator(int num);
void drawPasswordCorrect();
void drawPasswordWrong();
void drawAlcTestInfo();
void drawAlcTestFail();
void drawAlcTestPass();
void drawSudoOpen();
void drawNoPwdEeprom();

#endif
