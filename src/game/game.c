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
#include "projectile.h"
#include "decorations.h"
#include "obstacle_map.h"

#define WEAPON_OFFSET (SOLDIER_W >> 2)

#define NUM_PLAYERS 2
int player_count = 2;
static const PlayerConfig p1_cfg = PLAYER1_CONFIG;
static const PlayerConfig p2_cfg = PLAYER2_CONFIG;

//use these pointers to check for game win
static Entity *g_p1 = NULL;
static Entity *g_p2 = NULL;

static int bg_drawn = 0; //draw full background once at startup

int game_winner = 0;

void game_init(){
    map_init(2);
    decoration_init(2);
    obstacle_map_init();
    /*Spawn Player 1*/
    //on the left, faces right
    g_p1 = spawn_entity(ENTITY_PLAYER);
    g_p1->player_cfg = &p1_cfg;
    player_init(g_p1, SPRITE_PLAYER, (short)0xDC14, &p1_cfg, 16 + TILE_W, 0);

    /*Spawn Player 2*/
    //on the right, faces left
    g_p2 = spawn_entity(ENTITY_PLAYER);
    g_p2->player_cfg = &p2_cfg;
    player_init(g_p2, SPRITE_PLAYER, (short)0xDC14, &p2_cfg, SCREEN_WIDTH - 16 - TILE_W - PLAYER_W, 1);
}

void update_game(int cur_buf){
    keyboard_update();

    /*Check collisions before updating*/
    /* Melee collision - check each player*/
    Entity *players[2] = {g_p1, g_p2};
    int player_count = 2;
    for (int i = 0; i < player_count; i++){
        Entity *attacker = players[i];
        if (!attacker->attack_s1 && !attacker->attack_s2) continue;
        
        int weapon_x, weapon_y, weapon_length;
        int weapon_height = SOLDIER_H;

        if (attacker->type == ENTITY_PLAYER){
            int weapon_x = 0;
            int weapon_y = 0; 
            //int weapon_length = SOLDIER_W - (PLAYER_HITBOX_OFFSET_X + attacker->hitbox_w);
            int weapon_length = 0;
            int weapon_height = SOLDIER_H;

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

            for (int j = 0; j < player_count; j++){
                if (j == i) continue;
                Entity *target = players[j];
                if (target->dying || target->blocking) continue;

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
    }

    /*Projectile collision - each arrow vs each player*/
    for(int i = 0; i < MAX_ENTITIES; i++){
        Entity *arrow = &entities[i];
        if(!arrow->active || arrow->type != ENTITY_PROJECTILE || arrow->pending_erase) continue;
        
        for(int j = 0; j < player_count; j++){
            Entity *target = players[j];
            if(arrow->owner == target) continue;
            if(target->dying || target->blocking) continue;

            if(arrow->hitbox_x + arrow->hitbox_w >= target->hitbox_x &&
                arrow->hitbox_x <= target->hitbox_x + target->hitbox_w &&
                arrow->hitbox_y + arrow->hitbox_h >= target->hitbox_y &&
                arrow->hitbox_y <= target->hitbox_y + target->hitbox_h){

                target->was_hit = 1;
                target->damage = PROJECTILE_DAMAGE;

                //now need to erase projectile
                arrow->pending_erase = 1;
                arrow->pending_erase_b1 = 1;
                arrow->pending_erase_b2 = 1;
            }
        }
        
    }

    entity_update_all(cur_buf);

    /* Now spawn projectiles*/
    for (int i = 0; i < player_count; i++){
        Entity *p = players[i];
        if(!p->attack_p) continue;

        Entity *arrow = spawn_entity(ENTITY_PROJECTILE);
        if (arrow)
            projectile_init(arrow, p, SPRITE_PROJECTILE,
                            p->hitbox_x, p->hitbox_y, p->facing);
        p->attack_p = 0;
    }

    if(game_winner == 0){
        int alive_count = 0;
        int dying_count = 0;
        Entity *last_alive = NULL;

        for(int i = 0; i < player_count; i++){
            if (!players[i]->dying && players[i]->health > 0){
                alive_count++;
                last_alive = players[i];
            }
            else if (players[i]->dying){
                dying_count++;
            }
        }

        /* Only when all dying animations are done (active goes false) */
        if (alive_count + dying_count < player_count){
            if      (alive_count == 1) game_winner = (last_alive == g_p1) ? 1 :
                                                    (last_alive == g_p2) ? 2 : 4;
            else if (alive_count == 0) game_winner = 3;  /* draw */
        }
    }
}

void draw_game(int cur_buf){
    if(game_winner > 0){
        //write to character buffer
        volatile char *char_buf = (volatile char *)FPGA_CHAR_BASE;

        //clear a region in the middle
        const char *msg =
            game_winner == 1 ? " PLAYER 1 WINS" :
            game_winner == 2 ? " PLAYER 2 WINS" :
                               "     DRAW!    ";

        int row = 7;
        int col = 8;
        for (int i = 0; msg[i] != '\0'; i++){
            char_buf[row * 80 + col + i] = msg[i];
        }
    }
    if(!bg_drawn){
        //draw background into both buffers
        draw_background();
        decoration_draw_all();
        wait_for_vsync();
        draw_background();
        decoration_draw_all();
        bg_drawn = 1;
        return;
    }

    /* Erase each entity first */
    int px1 = 0, py1 = 0, px2 = 0, py2 = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active || entities[i].type != ENTITY_PLAYER) continue;
        if (entities[i].player_cfg == &p1_cfg) { px1 = entities[i].x; py1 = entities[i].y; }
        else                                   { px2 = entities[i].x; py2 = entities[i].y; }
    }

    entity_erase_all(cur_buf);
    entity_draw_all();
    decoration_draw_canopies_near(px1, py1, px2, py2);
}
