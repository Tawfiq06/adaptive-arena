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

    /*Check collisions */
    for (int i = 0; i < MAX_ENTITIES; i++){
        Entity *current = &entities[i];
        int x = current->x;
        int y = current->y; 
        char dir = current->facing;
        
        if (current->type == ENTITY_PLAYER){
            int weapon_x = 0;
            int weapon_y = 0; 
            int weapon_length = SOLDIER_W - (HITBOX_OFFSET_X + current->hitbox_w);
            int weapon_height = SOLDIER_H;
            
            if (current->attack_s1 | current->attack_s2) {
                /* We need too consider both directions because bounds change */
                if (dir == 'e'){    // if facing right, check right weapon coordinates 
                    weapon_x = x + current->hitbox_x + current->hitbox_w;     // weapon hit area is between weapon_x and x+weapon_length 
                    weapon_y = y;
                    /* check if any player was hit */
                    for(int j = 0; j < MAX_ENTITIES; j++){
                        if(j == i) //check that it is not the attacking player
                            continue;
                        Entity *temp = &entities[j];
                        if(temp->type != ENTITY_PLAYER)
                            continue;

                        if(temp->hitbox_x + temp->hitbox_w >= weapon_x 
                            && temp->hitbox_x <= weapon_x + weapon_length 
                            && temp->hitbox_y + temp->hitbox_h >= weapon_y 
                            && temp->hitbox_y <= weapon_y + weapon_height){
                            temp->was_hit = 1;
                            if(current->attack_s1){
                                temp->damage = ATTACK_1_DAMAGE;
                                current->attack_s1 = 0;
                            }
                            else if(current->attack_s2){
                                temp->damage = ATTACK_2_DAMAGE;
                                current->attack_s2 = 0;
                            }
                        }
                        //now update the player to handle that it was hit
                        player_update(temp, cur_buf);
                    }
                }
                if (dir == 'w'){    // if facing left, check left weapon coordinates 
                    weapon_x = x + weapon_length;
                    weapon_y = y;

                    /* check if any player was hit */
                    for(int j = 0; j < MAX_ENTITIES; j++){
                        if(j == i) //check that it is not the attacking player
                            continue;
                        Entity *temp = &entities[j];
                        if(temp->type != ENTITY_PLAYER)
                            continue;

                        if(temp->hitbox_x + temp->hitbox_w >= x 
                            && temp->hitbox_x <= weapon_x 
                            && temp->hitbox_y + temp->hitbox_h >= weapon_y 
                            && temp->hitbox_y <= weapon_y + weapon_height){
                            temp->was_hit = 1;
                            if(current->attack_s1){
                                temp->damage = ATTACK_1_DAMAGE;
                                current->attack_s1 = 0;
                            }
                            else if(current->attack_s2){
                                temp->damage = ATTACK_2_DAMAGE;
                                current->attack_s2 = 0;
                            }
                        }
                        //now update the player to handle that it was hit
                        player_update(temp, cur_buf);
                    }
                }
            }
            //Implement player launch projectile here
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
