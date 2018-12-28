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

#endif
