#ifndef UI_H
#define UI_H

enum uiStates
{
    ST_IDLE,
    ST_MIDICHSELECT,
    ST_SFSELECT,
    ST_MAX_STATES
};

int UI_init();
void UI_Task();
void UI_writeMessageToOLED(int xOffset, int lineNr, const char *text);
void UI_clearOLED();
int UI_checkRotary();
int UI_checkButton();
void UI_dispose();

#endif
