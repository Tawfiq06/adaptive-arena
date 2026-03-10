#include "vga.h"
#include "renderer.h"

int x = 50;
int dx = 2;

void game_init(){

}

void update_game(){
    x += dx;

    if (x < 0 || x > 300){
        dx = -dx;
    }
}

void draw_game(){
    fill_screen(0xFFFF);
    draw_rect(x, 100, 20, 20, 0xF800);
}