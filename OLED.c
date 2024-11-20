#include "OLED.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>

static const unsigned char font6x8_basic[128][6] = {
    // Define your 6x8 font here
    // Each character is represented by 6 bytes, each byte is a row of the character
    // For simplicity, only a few characters are defined here
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    [65] = {0x3E, 0x09, 0x09, 0x09, 0x3E, 0x00}, // A
    [66] = {0x3F, 0x25, 0x25, 0x25, 0x1A, 0x00}, // B
    [67] = {0x1E, 0x21, 0x21, 0x21, 0x12, 0x00}, // C
    [68] = {0x3F, 0x21, 0x21, 0x21, 0x1E, 0x00}, // D
    [69] = {0x3F, 0x25, 0x25, 0x25, 0x21, 0x00}, // E
    [70] = {0x3F, 0x05, 0x05, 0x05, 0x01, 0x00}, // F
    [71] = {0x1E, 0x21, 0x29, 0x29, 0x3A, 0x00}, // G
    [72] = {0x3F, 0x04, 0x04, 0x04, 0x3F, 0x00}, // H
    [73] = {0x00, 0x21, 0x3F, 0x21, 0x00, 0x00}, // I
    [74] = {0x10, 0x20, 0x21, 0x1F, 0x01, 0x00}, // J
    [75] = {0x3F, 0x04, 0x0A, 0x11, 0x20, 0x00}, // K
    [76] = {0x3F, 0x20, 0x20, 0x20, 0x20, 0x00}, // L
    [77] = {0x3F, 0x02, 0x04, 0x02, 0x3F, 0x00}, // M
    [78] = {0x3F, 0x02, 0x04, 0x08, 0x3F, 0x00}, // N
    [79] = {0x1E, 0x21, 0x21, 0x21, 0x1E, 0x00}, // O
    [80] = {0x3F, 0x09, 0x09, 0x09, 0x06, 0x00}, // P
    [81] = {0x1E, 0x21, 0x29, 0x11, 0x2E, 0x00}, // Q
    [82] = {0x3F, 0x09, 0x09, 0x19, 0x26, 0x00}, // R
    [83] = {0x26, 0x25, 0x25, 0x25, 0x19, 0x00}, // S
    [84] = {0x01, 0x01, 0x3F, 0x01, 0x01, 0x00}, // T
    [85] = {0x1F, 0x20, 0x20, 0x20, 0x1F, 0x00}, // U
    [86] = {0x0F, 0x10, 0x20, 0x10, 0x0F, 0x00}, // V
    [87] = {0x1F, 0x20, 0x18, 0x20, 0x1F, 0x00}, // W
    [88] = {0x3B, 0x04, 0x04, 0x04, 0x3B, 0x00}, // X
    [89] = {0x03, 0x04, 0x3C, 0x04, 0x03, 0x00}, // Y
    [90] = {0x31, 0x29, 0x25, 0x23, 0x21, 0x00}, // Z
    [97] = {0x10, 0x2A, 0x2A, 0x2A, 0x3C, 0x00}, // a
    [98] = {0x3F, 0x24, 0x24, 0x24, 0x18, 0x00}, // b
    [99] = {0x1C, 0x22, 0x22, 0x22, 0x14, 0x00}, // c
    [100] = {0x18, 0x24, 0x24, 0x24, 0x3F, 0x00}, // d
    [101] = {0x1C, 0x2A, 0x2A, 0x2A, 0x04, 0x00}, // e
    [102] = {0x04, 0x3E, 0x05, 0x01, 0x00, 0x00}, // f
    [103] = {0x18, 0x25, 0x25, 0x25, 0x3E, 0x00}, // g
    [104] = {0x3F, 0x04, 0x04, 0x04, 0x38, 0x00}, // h
    [105] = {0x00, 0x24, 0x3D, 0x20, 0x00, 0x00}, // i
    [106] = {0x10, 0x20, 0x20, 0x1D, 0x00, 0x00}, // j
    [107] = {0x3F, 0x08, 0x14, 0x22, 0x00, 0x00}, // k
    [108] = {0x00, 0x21, 0x3F, 0x20, 0x00, 0x00}, // l
    [109] = {0x3E, 0x02, 0x3E, 0x02, 0x3C, 0x00}, // m
    [110] = {0x3E, 0x04, 0x02, 0x02, 0x3C, 0x00}, // n
    [111] = {0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00}, // o
    [112] = {0x3E, 0x0A, 0x0A, 0x0A, 0x04, 0x00}, // p
    [113] = {0x04, 0x0A, 0x0A, 0x0A, 0x3E, 0x00}, // q
    [114] = {0x3E, 0x04, 0x02, 0x02, 0x04, 0x00}, // r
    [115] = {0x24, 0x2A, 0x2A, 0x2A, 0x12, 0x00}, // s
    [116] = {0x02, 0x1F, 0x22, 0x20, 0x00, 0x00}, // t
    [117] = {0x1E, 0x20, 0x20, 0x20, 0x3E, 0x00}, // u
    [118] = {0x0E, 0x10, 0x20, 0x10, 0x0E, 0x00}, // v
    [119] = {0x1E, 0x20, 0x18, 0x20, 0x1E, 0x00}, // w
    [120] = {0x22, 0x14, 0x08, 0x14, 0x22, 0x00}, // x
    [121] = {0x03, 0x24, 0x24, 0x24, 0x1F, 0x00}, // y
    [122] = {0x22, 0x32, 0x2A, 0x26, 0x22, 0x00}, // z
    [48] = {0x1E, 0x29, 0x25, 0x23, 0x1E, 0x00}, // 0
    [49] = {0x00, 0x22, 0x3F, 0x20, 0x00, 0x00}, // 1
    [50] = {0x32, 0x29, 0x29, 0x29, 0x26, 0x00}, // 2
    [51] = {0x12, 0x21, 0x25, 0x25, 0x1A, 0x00}, // 3
    [52] = {0x0C, 0x0A, 0x09, 0x3F, 0x08, 0x00}, // 4
    [53] = {0x17, 0x25, 0x25, 0x25, 0x19, 0x00}, // 5
    [54] = {0x1E, 0x25, 0x25, 0x25, 0x18, 0x00}, // 6
    [55] = {0x01, 0x01, 0x39, 0x05, 0x03, 0x00}, // 7
    [56] = {0x1A, 0x25, 0x25, 0x25, 0x1A, 0x00}, // 8
    [57] = {0x06, 0x29, 0x29, 0x29, 0x1E, 0x00}, // 9
    [33] = {0x00, 0x00, 0x2F, 0x00, 0x00, 0x00}, // !
    [34] = {0x00, 0x03, 0x00, 0x03, 0x00, 0x00}, // "
    [35] = {0x14, 0x3E, 0x14, 0x3E, 0x14, 0x00}, // #
    [36] = {0x12, 0x2A, 0x3F, 0x2A, 0x24, 0x00}, // $
    [37] = {0x22, 0x10, 0x08, 0x04, 0x22, 0x00}, // %
    [38] = {0x14, 0x2A, 0x2A, 0x14, 0x28, 0x00}, // &
    [39] = {0x00, 0x00, 0x03, 0x00, 0x00, 0x00}, // '
    [40] = {0x00, 0x1C, 0x22, 0x00, 0x00, 0x00}, // (
    [41] = {0x00, 0x22, 0x1C, 0x00, 0x00, 0x00}, // )
    [42] = {0x14, 0x08, 0x3E, 0x08, 0x14, 0x00}, // *
    [43] = {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // +
    [44] = {0x00, 0x20, 0x10, 0x00, 0x00, 0x00}, // ,
    [45] = {0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
    [46] = {0x00, 0x30, 0x30, 0x00, 0x00, 0x00}, // .
    [47] = {0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // /
    [58] = {0x00, 0x14, 0x14, 0x00, 0x00, 0x00}, // :
    [59] = {0x00, 0x20, 0x14, 0x00, 0x00, 0x00}, // ;
    [60] = {0x08, 0x14, 0x22, 0x00, 0x00, 0x00}, // <
    [61] = {0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // =
    [62] = {0x22, 0x14, 0x08, 0x00, 0x00, 0x00}, // >
    [63] = {0x02, 0x01, 0x29, 0x09, 0x06, 0x00}, // ?
    [64] = {0x1C, 0x22, 0x2D, 0x2D, 0x0E, 0x00}, // @
    [91] = {0x00, 0x3E, 0x22, 0x00, 0x00, 0x00}, // [
    [92] = {0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // Backslash
    [93] = {0x00, 0x22, 0x3E, 0x00, 0x00, 0x00}, // ]
    [94] = {0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
    [95] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x00}, // _
    [96] = {0x00, 0x01, 0x02, 0x00, 0x00, 0x00}, // `
    [123] = {0x00, 0x08, 0x36, 0x22, 0x00, 0x00}, // {
    [124] = {0x00, 0x00, 0x3E, 0x00, 0x00, 0x00}, // |
    [125] = {0x00, 0x22, 0x36, 0x08, 0x00, 0x00}, // }
    [126] = {0x04, 0x02, 0x04, 0x08, 0x04, 0x00}  // ~
};

int OLED_fbFd = 0;
uint8_t *OLED_fbp = 0;
size_t OLED_fbSize = 0;
struct fb_var_screeninfo OLED_vinfo;
struct fb_fix_screeninfo OLED_finfo;

void OLED_drawText6x8(int x, int y, const char *text)
{
    int location;
    for (int i = 0; text[i] != '\0'; i++)
    {
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 6; col++)
            {
                if (font6x8_basic[(int)text[i]][col] & (1 << row))
                {
                    location = (x + col + (i * 6) + OLED_vinfo.xoffset) + ((y + row + OLED_vinfo.yoffset) * OLED_vinfo.xres);
                    OLED_fbp[location*2] = 0xff; // Set pixel to white
                    OLED_fbp[location*2+1] = 0xff;
                }
            }
        }
    }
}

uint8_t* OLED_init()
{
    OLED_fbFd = open("/dev/fb0", O_RDWR);
    if (OLED_fbFd == -1)
    {
        printf("[ERROR] Kon fb0 niet openen.\n");
        printf("        Staat de overlay ssd1306-spi aan in /boot/firmware/config.txt?\n");
        exit(EXIT_FAILURE);
    }

    if (ioctl(OLED_fbFd, FBIOGET_VSCREENINFO, &OLED_vinfo))
    {
        perror("[ERROR] Error reading variable information");
        close(OLED_fbFd);
        exit(EXIT_FAILURE);
    }
    if (ioctl(OLED_fbFd, FBIOGET_FSCREENINFO, &OLED_finfo))
    {
        perror("[ERROR] Error reading fixed information");
        close(OLED_fbFd);
        exit(EXIT_FAILURE);
    }
    OLED_fbSize = OLED_vinfo.yres_virtual * OLED_vinfo.xres_virtual * 2; // 16 bit per pixel
    printf("[INFO] Virtuele schermbreedte: %d, hoogte: %d.\n", OLED_vinfo.xres_virtual, OLED_vinfo.yres_virtual);

    OLED_fbp = (uint8_t *)mmap(0, OLED_fbSize, PROT_READ | PROT_WRITE, MAP_SHARED, OLED_fbFd, 0);
    if ((intptr_t)OLED_fbp == -1)
    {
        perror("[ERROR] Error mapping framebuffer device to memory");
        close(OLED_fbFd);
        exit(EXIT_FAILURE);
    }
    return OLED_fbp;
}

void OLED_clear()
{
    memset(OLED_fbp, 0, OLED_fbSize);
}

void OLED_dispose()
{
    munmap(OLED_fbp, OLED_fbSize);
    close(OLED_fbFd);
    OLED_fbFd = 0;
    OLED_fbSize = 0;
}