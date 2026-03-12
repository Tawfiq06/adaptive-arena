#include "vga.h"
#include "renderer.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include <stdlib.h>

Entity entities[MAX_ENTITIES];

void game_init(){
    map_init(1);
    player_init(&entities[0], SPRITE_PLAYER, (short) 0xDC14);
}

void update_game(){
    entity_update_all();
}

void draw_game(){
    draw_background();
    entity_draw_all();
}