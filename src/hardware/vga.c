#include "vga.h"

//define to make setting colour easier to write
//  get address =  base + (y * 1024) + (x * 2)
//  y * 1024 -> buffer width is 512, but each pixel is 2 bytes so 512 * 2 = 1024
#define PIXEL(x,y) (*(volatile short *)(PIXEL_BASE + ((y)<<10) + ((x)<<1)))
// ^^ compiler will replace PIXEL(x,y) with the define

void plot_pixel(int x, int y, short colour){
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
        return;

    volatile short *addr = 
        (volatile short *)(PIXEL_BASE + (y << 10) + (x << 1));

    *addr = colour;
}

void fill_screen(short colour){
    for (int y = 0; y < SCREEN_HEIGHT; y++){
        //comput start of row once (faster then using the plot pixel function)
        volatile short *row_ptr = (volatile short*)(PIXEL_BASE + (y << 10));

        for (int x = 0; x < SCREEN_WIDTH; x++){
            row_ptr[x] = colour;
        }
    }
}

void draw_rect(int x, int y, int width, int height, short colour){
    int x_end = x + width;
    int y_end = y + height;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x_end > SCREEN_WIDTH) x_end = SCREEN_WIDTH;
    if (y_end > SCREEN_HEIGHT) y_end = SCREEN_HEIGHT;
    
    for(int row = y; row < y_end; row++){
        volatile short *row_ptr = (volatile short*)(PIXEL_BASE + (row << 10));
        for(int col = x ; col < x_end; col++){
            row_ptr[col] = colour;
        }
    }
}