#pragma once
#include <stdint.h>

#define NUM_DISPLAY 3
#define TIME_MUX 4

void Display7seg_init(void);
void Display7seg_refresh(void);
void Display7seg_show_number(uint16_t numero);
void Display7seg_show_text(const char *texto);
