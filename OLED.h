#ifndef OLED_H
#define OLED_H

#include <stdint.h>

void OLED_drawText6x8(int x, int y, const char *text);
void OLED_writeLine(int xOffset, int lineNr, const char *text);
uint8_t* OLED_init();
void OLED_clear();
void OLED_dispose();

#endif
