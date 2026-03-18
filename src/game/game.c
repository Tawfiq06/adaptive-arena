#include "vga.h"
#include "renderer.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include <stdlib.h>
#include "keyboard.h"
#include "player_config.h"
#include "tile_sprites.h"
#include "soldier_frames.h"

static const PlayerConfig p1_cfg = PLAYER1_CONFIG;
static const PlayerConfig p2_cfg = PLAYER2_CONFIG;

static int bg_drawn = 0; //draw full background once at startup

void game_init(){
    map_init(1);

    /*Spawn Player 1*/
    //on the left, faces right
    Entity *p1 = spawn_entity(ENTITY_PLAYER);
    p1->player_cfg = &p1_cfg;
    player_init(p1, SPRITE_PLAYER, (short)0xDC14, &p1_cfg, 16 + TILE_W, 0);

    /*Spawn Player 2*/
    //on the right, faces left
    Entity *p2 = spawn_entity(ENTITY_PLAYER);
    p2->player_cfg = &p2_cfg;
    player_init(p2, SPRITE_PLAYER, (short)0xDC14, &p2_cfg, SCREEN_WIDTH - 16 - TILE_W - PLAYER_W, 1);
}

void update_game(int cur_buf){
    keyboard_update();
    entity_update_all(cur_buf);

    for (int i = 0; i < MAX_ENTITIES; i++){
        Entity current = entities[i];
        int x = current.x;
        int y = current.y; 
        char dir = current.facing;
        
        if (current.type == ENTITY_PLAYER){
            int weapon_x = 0;
            int weapon_y = 0; 
            int weapon_length = SOLDIER_W - (hitbox_x + hitbox_w);
            int weapon_height = SOLDER_H;
            
            if (current.attack_s1 | current.attack_s2) {
                if (dir == 'e'){    // if facing right, check right weapon coordinates 
                    weapon_x = x + hitbox_x + hitbox_w;     // weapon hit area is between weapon_x and x+weapon_length 
                    weapon_y = y;
                }
                if (dir == 'w'){    // if facing left, check left weapon coordinates 
                    weapon_x = x + weapon_length;
                    weapon_y = y;
                }
            }
            
            }
    }
    
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
