#ifndef VGA_H
#define VGA_H

#include "address_map.h"

#define SCREEN_WIDTH 320 //visible widtg
#define SCREEN_HEIGHT 240
#define BUFFER_WIDTH 512 //actual memory width (power of 2)

#define PIXEL_BASE FPGA_PIXEL_BUF_BASE

void plot_pixel(int x, int y, short colour);
void fill_screen(short colour);

#endif