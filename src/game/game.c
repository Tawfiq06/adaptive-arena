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

#define WEAPON_OFFSET (SOLDIER_W >> 2)

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

    /*Check collisions before updating*/
    for (int i = 0; i < MAX_ENTITIES; i++){
        Entity *attacker = &entities[i];
        if(!attacker->active) continue;
        
        if (attacker->type == ENTITY_PLAYER){
            int weapon_x = 0;
            int weapon_y = 0; 
            //int weapon_length = SOLDIER_W - (PLAYER_HITBOX_OFFSET_X + attacker->hitbox_w);
            int weapon_length = 0;
            int weapon_height = SOLDIER_H;
            
            if (attacker->attack_s1 | attacker->attack_s2) {
                /* We need too consider both directions because bounds change */
                if(attacker->facing == 'e'){
                    /* Facing right: weapon extends from right edge of hitbox outward*/
                    weapon_x = attacker->hitbox_x + attacker->hitbox_w - WEAPON_OFFSET;
                    weapon_y = attacker->y;
                    weapon_length = (attacker->x + SOLDIER_W) - weapon_x;
                }
                else{
                    /*Facing left: weapon extends from left edge of sprite to left edge of hitbox*/
                    weapon_x = attacker->x;
                    weapon_y = attacker->y;
                    weapon_length = attacker->hitbox_x + WEAPON_OFFSET - attacker->x;
                }

                if(weapon_length <= 0) continue;

                for (int j = 0; j < MAX_ENTITIES; j++){
                    if (j == i) continue;
                    Entity *target = &entities[j];
                    if(!target->active) continue;
                    if(target->type != ENTITY_PLAYER) continue;
                    if (target->dying) continue;

                    if(target->hitbox_x + target->hitbox_w >= weapon_x 
                        && target->hitbox_x <= weapon_x + weapon_length 
                        && target->hitbox_y + target->hitbox_h >= weapon_y 
                        && target->hitbox_y <= weapon_y + weapon_height){

                            target->was_hit = 1;
                            if(attacker->attack_s1){
                                target->damage = ATTACK_1_DAMAGE;
                            }
                            else if(attacker->attack_s2){
                                target->damage = ATTACK_2_DAMAGE;
                            }

                            /*Clear flags*/
                            attacker->attack_s1 = 0;
                            attacker->attack_s2 = 0;
                    }
                }
            }
            //Implement player launch projectile here
        }
    }
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
