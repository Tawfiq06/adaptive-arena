#ifndef VGA_H
#define VGA_H

#include "address_map.h"

#define SCREEN_WIDTH 320 //visible widtg
#define SCREEN_HEIGHT 240
#define BUFFER_WIDTH 512 //actual memory width (power of 2)

#define PIXEL_BASE FPGA_PIXEL_BUF_BASE

void vga_init();
void plot_pixel(int x, int y, short int colour);
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int colour);
void wait_for_vsync();
void draw_rect(int x, int y, int width, int height, short colour);
void draw_rect_outline(int x, int y, int width, int height, short colour);
void fill_screen(short colour);

#endif