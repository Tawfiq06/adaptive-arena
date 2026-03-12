#include "vga.h"
#include <stdlib.h>
#include <stdbool.h>

/*Global pixel buffer pointer */
volatile int pixel_buffer_start; // global variable

/*Back buffers (for double buffer)*/
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];

/* Helpers Functions*/
static void swap(int* a, int* b);

void vga_init(){
    volatile int *pixel_ctrl_ptr = (volatile int*) PIXEL_BUF_CTRL_BASE;

    *pixel_ctrl_ptr = 0; //clear just in case
    /* Set front buffer to Buffer1*/
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;

    wait_for_vsync();

    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    /*set back buffer to Buffer2*/
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;

    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();
}

void plot_pixel(int x, int y, short int colour) {
  volatile short int* one_pixel_address;
  one_pixel_address = (volatile short int*)(pixel_buffer_start + (y << 10) + (x << 1));
  *one_pixel_address = colour;
}

void clear_screen() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    volatile short *row_ptr = (volatile short* )(pixel_buffer_start + (y << 10));

    for (int x = 0; x < SCREEN_WIDTH; x++) {
      row_ptr[x] = 0x0000;  // set to black
    }
  }
}

void fill_screen(short colour) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = colour;
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
        volatile short *row_ptr = (volatile short*)(pixel_buffer_start + (row << 10));
        for(int col = x ; col < x_end; col++){
            row_ptr[col] = colour;
        }
    }
}

static void swap(int* a, int* b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void draw_line(int x0, int y0, int x1, int y1, short int colour) {
  bool is_steep = abs(y1 - y0) > abs(x1 - x0);
  if (is_steep) {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }
  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }

  int deltax = x1 - x0;
  int deltay = abs(y1 - y0);
  int error = -(deltax / 2);
  int y = y0;
  int y_step = (y0 < y1) ? 1 : -1;

  for (int x = x0; x <= x1; x++) {
    if (is_steep) {
      plot_pixel(y, x, colour);
    } else {
      plot_pixel(x, y, colour);
    }
    error += deltay;
    if (error > 0) {
      y += y_step;
      error -= deltax;
    }
  }
}

void wait_for_vsync() {
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    *pixel_ctrl_ptr = 1;                  // request swap
    while (*(pixel_ctrl_ptr + 3) & 0x1);  // wait for S bit to go to
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);//update start
}
