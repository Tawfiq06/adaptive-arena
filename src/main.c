#include "address_map.h"
#include "vga.h"
#include "timer.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

int main(void){
    int x = 50;
    int y = 100;
    int size = 20;
    int dx = 2; // horizontal movement speed

    vga_init();
    timer_init();

    while(1){
        fill_screen(0xFFFF);

        draw_rect(x, y, size, size, 0xF800); // red square

        //update postion
        x += dx;

        // bounce off edges
        if(x <= 0 || x + size >= SCREEN_WIDTH){
            dx = -dx;
        }

        if(frame_flag){
            frame_flag = 0;

            update_game();
            draw_game();
            
            wait_for_vsync();
        }
    }

    return;
}