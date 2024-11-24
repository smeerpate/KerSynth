#ifndef UI_H
#define UI_H

int UI_init();
void UI_Task();
void UI_writeMessageToOLED(int xOffset, int lineNr, const char *text);
int UI_checkRotary();
int UI_checkButton();
void UI_dispose();

#endif
