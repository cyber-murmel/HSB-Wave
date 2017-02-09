#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

#define HUE_MAX ((1<<8)*6)

#pragma pack(1)
typedef struct rgb {
    uint8_t r, g, b;
} rgb_t;

typedef union color {
    rgb_t rgb;
    uint8_t u8[3];
} color_t;

/**
Translates a HSB value to a RGB color value
*/
void hsb_to_color(uint16_t hue, uint8_t sat, uint8_t val, color_t* color_p);

/**
Calculate value of faded hue; use shortest way around color wheel
*/
uint16_t inter_hue(uint16_t start, uint16_t stop, uint16_t step_num, uint16_t step_cur);

#endif
