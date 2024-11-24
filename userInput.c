// een rotary encoder wordt aangesloten op GPIO (BCM nummering) 17(A) en 27(B) en 22(drukken)
// overlay moet enabled zijn in /boot/firmware/config.txt
// dtoverlay=rotary-encoder,pin_a=17,pin_b=27,relative_axis=1
/*
+-----+-----+---------+------+---+---Pi 4---+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
|     |     |    3.3V |      |   |  1 || 2  |   |      | 5V      |     |     |
|   2 |   8 |   SDA1  | ALT0 | 1 |  3 || 4  |   |      | 5V      |     |     |
|   3 |   9 |   SCL1  | ALT0 | 1 |  5 || 6  |   |      | 0V      |     |     |
|   4 |   7 |  GPIO7  |  IN  | 0 |  7 || 8  | 1 | ALT0 | TxD     | 15  | 14  |
|     |     |      0V |      |   |  9 || 10 | 1 | ALT0 | RxD     | 16  | 15  |
|  17 |   0 |  GPIO0  |  IN  | 0 | 11 || 12 | 0 | IN   | GPIO1   | 1   | 18  |
|  27 |   2 |  GPIO2  |  IN  | 0 | 13 || 14 |   |      | 0V      |     |     |
|  22 |   3 |  GPIO3  |  IN  | 0 | 15 || 16 | 0 | IN   | GPIO4   | 4   | 23  |
|     |     |    3.3V |      |   | 17 || 18 | 0 | IN   | GPIO5   | 5   | 24  |
|  10 |  12 |  MOSI   | ALT0 | 1 | 19 || 20 |   |      | 0V      |     |     |
|   9 |  13 |  MISO   | ALT0 | 1 | 21 || 22 | 0 | IN   | GPIO6   | 6   | 25  |
|  11 |  14 |  SCLK   | ALT0 | 1 | 23 || 24 | 1 | ALT0 | CE0     | 10  | 8   |
|     |     |      0V |      |   | 25 || 26 | 1 | ALT0 | CE1     | 11  | 7   |
|   0 |  30 |  SDA0   |  IN  | 0 | 27 || 28 | 0 | IN   | SCL0    | 31  | 1   |
|   5 |  21 |  GPIO21 |  IN  | 0 | 29 || 30 |   |      | 0V      |     |     |
|   6 |  22 |  GPIO22 |  IN  | 0 | 31 || 32 | 0 | IN   | GPIO26  | 26  | 12  |
|  13 |  23 |  GPIO23 |  IN  | 0 | 33 || 34 |   |      | 0V      |     |     |
|  19 |  24 |  GPIO24 |  IN  | 0 | 35 || 36 | 0 | IN   | GPIO27  | 27  | 16  |
|  26 |  25 |  GPIO25 |  IN  | 0 | 37 || 38 | 0 | IN   | GPIO28  | 28  | 20  |
|     |     |      0V |      |   | 39 || 40 | 0 | IN   | GPIO29  | 29  | 21  |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
*/
#include "userInput.h"
#include "OLED.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <errno.h>

const char *button = "/dev/input/event0";
const char *rotEncoder = "/dev/input/event1"; // Identify the Input Device with ls /dev/input/ (Look for a device like /dev/input/eventX where X is the event number)
struct input_event rotaryEncEv;
struct input_event buttonEv;
int rotaryEncFd = -1;
int buttonFd = -1;
uiState_t uiState = ST_IDLE;

int UI_init()
{
    buttonFd = open(button, O_RDONLY | O_NONBLOCK);
    if (buttonFd == -1)
    {
        printf("[ERROR] kon button niet openen.\n");
        return EXIT_FAILURE;
    }

    rotaryEncFd = open(rotEncoder, O_RDONLY | O_NONBLOCK);
    if (rotaryEncFd == -1)
    {
        printf("[ERROR] Failed to open input for rotary encoder\n");
        return EXIT_FAILURE;
    }
    
    OLED_init();
    OLED_clear();
    OLED_writeLine(5, 0, "UI init done...");
    printf("[INFO] User input init klaar.\n");
    return 0;
}

void UI_Task()
{
    UI_checkRotary();
    UI_checkButton();
    
    switch (uiState)
    {
        case ST_IDLE:
            break;
        
        case ST_MIDICHSELECT:
            break;
            
        default:
            break;
    }
}

void UI_writeMessageToOLED(int xOffset, int lineNr, const char *text)
{
    OLED_writeLine(xOffset, lineNr, text);
}

void UI_clearOLED()
{
    OLED_clear();
}

int UI_checkRotary()
{
    int verbose = 1;

    if (rotaryEncFd != -1)
    {
        ssize_t bytes = read(rotaryEncFd, &rotaryEncEv, sizeof(rotaryEncEv));
        if (bytes == -1)
        {
            if (errno == EAGAIN)
            {
                return 0;
            }
            else
            {
                printf("[ERROR] Failed to read input event\n");
            close(rotaryEncFd);
                rotaryEncFd = -1;
            return EXIT_FAILURE;
            }
        }

        if (verbose > 0) printf("[INFO] Aantal encoder event bytes gelezen = %d\n", bytes);
        // Check for rotary encoder events
        if (rotaryEncEv.type == EV_REL && rotaryEncEv.code == REL_X)
        {
            printf("[INFO] Rotary Encoder Value: %d\n", rotaryEncEv.value);
        }
    }
    else
    {
        printf("[WARNING] Geen file descriptor voor user input\n");
    }
    return 0;
}

int UI_checkButton()
{
    int verbose = 1;

    if (buttonFd != -1)
    {
        ssize_t bytes = read(buttonFd, &buttonEv, sizeof(buttonEv));
        if (bytes == -1)
        {
            if (errno == EAGAIN)
            {
                return 0;
            }
            else
            {
                printf("[ERROR] kon input event niet lezen\n");
                close(buttonFd);
                return EXIT_FAILURE;
            }
        }

        if (verbose > 0) printf("[INFO] Aantal button event bytes gelezen = %d\n", bytes);
        if (buttonEv.type == EV_KEY)
        {
            printf("Key %d %s\n", buttonEv.code, buttonEv.value ? "pressed" : "released");
        }
    }
    else
    {
        printf("[WARNING] Geen file descriptor voor user input\n");
    }
    return 0;
}

void UI_dispose()
{
    OLED_dispose();
}
