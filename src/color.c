#include <stdint.h>
#include <stdlib.h>
#include "color.h"

void hsb_to_color(uint16_t hue, uint8_t sat, uint8_t val, color_t* color_p) {
  uint16_t rel_hue = hue % 512;
  uint8_t seg, i, idx;
  uint8_t rgb[3];
  if(sat > 100)
    sat = 100;
  if(val > 100)
    val = 100;
  rgb[0] = (rel_hue < 256) ? 255 : 255 - (hue % 256);
  rgb[1] = (rel_hue < 256) ? (hue % 256) : 255;
  rgb[2] = 0;
  seg = hue/512;
  for(i = 0; i < 3; i++) {
    idx = (i+seg)%3;
    (*color_p).u8[idx] = rgb[i];
    (*color_p).u8[idx] = ((255*(100-sat))/100) + (((*color_p).u8[idx]*sat)/100);
    (*color_p).u8[idx] = ((*color_p).u8[idx]*val)/100;
  }
}


/*
             | dist < HUE_MAX/2 |dist > HUE_MAX/2
-------------+------------------+-----------------
start < stop | start -> stop    | <- start ... stop <-
-------------+------------------+-----------------
start > stop | stop <- start    | -> stop ... start ->
*/
uint16_t inter_hue(uint16_t start_val, uint16_t stop_val, uint16_t step_max, uint16_t step_cur) {
  int32_t result;
  uint32_t dist = abs(stop_val - start_val);
  
  if((start_val <= stop_val)) {
    if((dist <= HUE_MAX/2)){
      /* go forward from start to stop
         no wrap aound */
      result = start_val + ((dist*step_cur)/step_max);
    }
    else {
      /* go backward from start to stop
         wrap aound */
      dist = HUE_MAX - dist;
      result = start_val - ((dist*step_cur)/step_max);
    }
  }
  else {
    if((dist <= HUE_MAX/2)){
      /* go backward from start to stop
         no wrap aound */
      result = start_val - ((dist*step_cur)/step_max);
    }
    else {
      /* go forward from start to stop
         wrap aound */
      dist = HUE_MAX - dist;
      result = start_val + ((dist*step_cur)/step_max);
    }
  }
  result = (result+HUE_MAX)%HUE_MAX;
  return (int32_t)result;
}