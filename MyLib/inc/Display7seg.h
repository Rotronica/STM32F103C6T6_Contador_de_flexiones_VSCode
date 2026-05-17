#pragma once
#include <stdint.h>

#define NUM_DISPLAY 3
#define TIME_MUX 6

void Display7seg_init(void);
void Display_refresh(void);
void Display7_show_number(uint16_t numero);
