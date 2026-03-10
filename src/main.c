#include "address_map.h"
#include "vga.h"
#include "timer.h"
#include "game.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

int main(void){

    vga_init();
    timer_init();
    game_init();

    while(1){

        if(frame_flag){
            frame_flag = 0;

            update_game();
            draw_game();
            
            wait_for_vsync(); //display frame
        }
    }

    return 0;
}