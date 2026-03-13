#include "vga.h"
#include "renderer.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include <stdlib.h>
#include "keyboard.h"

Entity entities[MAX_ENTITIES];

static int bg_drawn = 0; //draw full background once at startup
void game_init(){
    map_init(1);
    player_init(&entities[0], SPRITE_PLAYER, (short) 0xDC14);
}

void update_game(int cur_buf){
    keyboard_update();
    entity_update_all(cur_buf);
}

void draw_game(int cur_buf){
    if(!bg_drawn){
        draw_background();
        wait_for_vsync();
        draw_background();
        bg_drawn = 1;
        return;
    }

    /* Erase each entity first */
    entity_erase_all(cur_buf);
    entity_draw_all();
}