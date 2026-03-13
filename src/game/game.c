#include "vga.h"
#include "renderer.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include <stdlib.h>
#include "keyboard.h"
#include "player_config.h"

Entity entities[MAX_ENTITIES];

static const PlayerConfig p1_cfg = PLAYER1_CONFIG;
static const PlayerConfig p2_cfg = PLAYER2_CONFIG;

static int bg_drawn = 0; //draw full background once at startup

void game_init(){
    map_init(1);

    /*Spawn Player 1*/
    Entity *p1 = spawn_entity(ENTITY_PLAYER);
    p1->player_cfg = &p1_cfg;
    player_init(p1, SPRITE_PLAYER, (short)0xDC14, &p1_cfg);
    p1->x = 60;
    p1->prev_x[0] = p1->x;
    p1->prev_x[1] = p1->x;

     /*Spawn Player 2*/
    Entity *p2 = spawn_entity(ENTITY_PLAYER);
    p2->player_cfg = &p2_cfg;
    player_init(p2, SPRITE_PLAYER, (short)0xDC14, &p2_cfg);
    p2->x = 60;
    p2->prev_x[0] = p2->x;
    p2->prev_x[1] = p2->x;
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