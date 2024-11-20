/*
  compileren: gcc -o clear_fb clear_fb.c
  uirvoeren: sudo ./clear_fb
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>


int main() {
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        perror("Error opening framebuffer device");
        exit(EXIT_FAILURE);
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        close(fb_fd);
        exit(EXIT_FAILURE);
    }
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Error reading fixed information");
        close(fb_fd);
        exit(EXIT_FAILURE);
    }

    size_t screensize = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;
    uint8_t *fbp = (uint8_t *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if ((intptr_t)fbp == -1) {
        perror("Error mapping framebuffer device to memory");
        close(fb_fd);
        exit(EXIT_FAILURE);
    }

    printf("======== fb_var_screeninfo ========\n");
    printf("[INFO] Visuele resolutie: breedte: %d, hoogte: %d.\n", vinfo.xres, vinfo.yres);
    printf("[INFO] Virtuele resolutie: breedte: %d, hoogte: %d.\n", vinfo.xres_virtual, vinfo.yres_virtual);
    printf("[INFO] Offset: x: %d, y: %d.\n", vinfo.xoffset, vinfo.yoffset);
    printf("[INFO] Bits per pixel: %d.\n", vinfo.bits_per_pixel);
    printf("[INFO] Grayscale: %d.\n", vinfo.grayscale);
    printf("[INFO] height, width in mm: %d, %d.\n", vinfo.height, vinfo.width);

    printf("[INFO] Screensize: %d\n", screensize);

    // Clear the framebuffer by setting all bytes to zero
    memset(fbp, 0, screensize/2);
    memset(fbp + screensize/2 - 1, 0x00, screensize/2);

    for (int pixCntr = screensize/4; pixCntr < screensize/2; pixCntr++)
    {
        if(pixCntr%2 == 0)
        {
            *(fbp+(pixCntr*2)) = 0xff;
            *(fbp+(pixCntr*2)+1) = 0xff;
        }
        else
        {
            *(fbp+(pixCntr*2)) = 0x00;
            *(fbp+(pixCntr*2)+1) = 0x00;
        }
    }

    // Unmap and close the framebuffer device
    munmap(fbp, screensize);
    close(fb_fd);

    // Write a message to the console
    printf("[INFO] Framebuffer gezet.\n");

    return 0;
}
