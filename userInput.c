// een rotary encoder wordt aangesloten op GPIO (BCM nummering) 5(A) en 6(B) en 16(drukken)
// overlay moet enabled zijn in /boot/firmware/config.txt
// dtoverlay=rotary-encoder,pin_a=5,pin_b=6,relative_axis=1
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>

#define GPIO_BASE 0x3F200000

const char *device = "/dev/input/event0"; // Identify the Input Device with ls /dev/input/ (Look for a device like /dev/input/eventX where X is the event number)
struct input_event ev;
int fd;

void UI_enablePullUp(int gpio)
{
    volatile unsigned int *gpio_base;
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) 
	{
        perror("open");
        exit(EXIT_FAILURE);
    }

    gpio_base = (unsigned int *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE);
    if (gpio_base == MAP_FAILED)
	{
        perror("mmap");
        close(mem_fd);
        exit(EXIT_FAILURE);
    }

    // Enable pull-up resistor
    *(gpio_base + (gpio / 10)) |= (1 << ((gpio % 10) * 3));
    close(mem_fd);
}

int UI_init()
{
	UI_enablePullUp(5); // pullup for A
	UI_enablePullUp(6); // pullup for B
	UI_enablePullUp(16); // pullup for pushbutton
	fd = open(device, O_RDONLY);
    if (fd == -1)
	{
        perror("Failed to open input device");
        return EXIT_FAILURE;
    }
	return 0;
}

int UI_checkRotary()
{
	if (read(fd, &ev, sizeof(struct input_event)) == -1)
	{
		perror("Failed to read input event");
		close(fd);
		return EXIT_FAILURE;
	}

	// Check for rotary encoder events
	if (ev.type == EV_REL && ev.code == REL_X)
	{
		printf("Rotary Encoder Value: %d\n", ev.value);
	}
	return 0;
}

