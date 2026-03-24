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
int game_timer = 0;

/* Potion stuff */
#define MAX_POTIONS 2
#define POTION_SPAWN_INTERVAL 1800 /* 30s at 60 fps*/
#define POTION_HEAL 25
#define POTION_LIFETIME 1800 /* 30 Seconds */

typedef struct{
    int x, y;
    int active;
    int lifetime;
    int erase_b0;
    int erase_b1;
    Sprite sprite;
    int flash_drawn;
    int needs_draw_b0;
    int needs_draw_b1;
} Potion;

Potion potions[MAX_POTIONS];
int next_potion_spawn = 900; /*first one at 15s*/

/* Storm cloud */
#define MAX_CLOUD_TILES (MAP_WIDTH * MAP_HEIGHT)
#define STORM_START_TIME  1800
#define STORM_TILE_INTERVAL 1800

typedef struct {
    short row, col;
    int draw_b0;
    int draw_b1;
    Sprite sprite;
} CloudTile;

CloudTile cloud_tiles[MAX_CLOUD_TILES];

int cloud_tile_count = 0;
int storm_ring = 0;
int next_storm_tick = STORM_START_TIME;
int storm_damage = 3;
int max_rings = 6;

void add_cloud_tile(int row, int col){
    if (cloud_tile_count >= MAX_CLOUD_TILES) return;
    cloud_tiles[cloud_tile_count].row    = (short)row;
    cloud_tiles[cloud_tile_count].col    = (short)col;
    cloud_tiles[cloud_tile_count].draw_b0 = 1;
    cloud_tiles[cloud_tile_count].draw_b1 = 1;
    cloud_tiles[cloud_tile_count].sprite = sprites[SPRITE_POSION_CLOUD];
    cloud_tile_count++;
    obstacle_map_set(row, col, TILE_FLAG_DAMAGE | TILE_FLAG_SLOW);
}

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

    for (int i = 0; i < MAX_POTIONS; i++) {
        potions[i].active     = 0;
        potions[i].erase_b0   = 0;
        potions[i].erase_b1   = 0;
        potions[i].flash_drawn = 0;
    }
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
            else if (players[i]->dying && players[i]->active){
                dying_count++;
            }
        }

        /* Only when all dying animations are done (active goes false) */
        if (alive_count < player_count && dying_count == 0){
            if (alive_count == 1) game_winner = (last_alive == g_p1) ? 1 :
                                                    (last_alive == g_p2) ? 2 : 4;
            else if (alive_count == 0) game_winner = 3;  /* draw */
        }
    }

    /*now handle potion stuff*/
    game_timer++;

    /*Spawn Potion*/
    if(game_timer == next_potion_spawn){
        for(int i = 0; i < MAX_POTIONS; i++){
            if(!potions[i].active){
                /*try random tile until we find a good one*/
                int placed = 0;
                for (int attempts = 0; attempts < 100; attempts++){
                    int tile_col = 1 + rand() % (MAP_WIDTH - 2);
                    int tile_row = 1 + rand() % (MAP_HEIGHT - 2);

                    //check the tile is safe
                    unsigned char flags = obstacle_map_get(tile_row, tile_col);
                    if(flags & (TILE_FLAG_SOLID | TILE_FLAG_DAMAGE)) continue;

                    potions[i].active = 1;
                    potions[i].lifetime = POTION_LIFETIME;
                    potions[i].sprite = sprites[SPRITE_LIFE_POT];
                    potions[i].erase_b0 = 0;
                    potions[i].erase_b1 = 0;
                    potions[i].flash_drawn = 0;
                    potions[i].needs_draw_b0 = 1;
                    potions[i].needs_draw_b1 = 1;

                    potions[i].x = tile_col * TILE_W + rand() % (TILE_W - potions[i].sprite.width);
                    potions[i].y = tile_row * TILE_H + rand() % (TILE_H - potions[i].sprite.height);
                    next_potion_spawn = game_timer + POTION_SPAWN_INTERVAL;
                    placed = 1;
                    break;
                }
                if (placed) break;
            }
        }
    }

    /* Check pickup */
    for(int i = 0; i < MAX_POTIONS; i++){
        if (!potions[i].active) continue;

        potions[i].lifetime--;
        if(potions[i].lifetime <= 0){
            potions[i].active = 0;
            potions[i].erase_b0 = 1;
            potions[i].erase_b1 = 1;
            continue; //skip pickup it is gone now
        }

        for(int j = 0; j < player_count; j++){
            Entity *p = players[j];
            if(!p->active || p->dying) continue;
            if(p->hitbox_x < potions[i].x + potions[i].sprite.width &&
               p->hitbox_x + p->hitbox_w > potions[i].x &&
               p->hitbox_y < potions[i].y + potions[i].sprite.height &&
               p->hitbox_y + p->hitbox_h > potions[i].y){
                p->health += POTION_HEAL;
                if(p->health > HEALTH) p->health = HEALTH;
                potions[i].active = 0;
                potions[i].erase_b0 = 1;
                potions[i].erase_b1 = 1;
                break;
               }
        }
    }

    /* Storm cloud */
    if(game_timer == next_storm_tick && storm_ring < max_rings){
        int r_top = storm_ring; //top row
        int r_bottom = MAP_HEIGHT - 1 - storm_ring; //bottom row

        int c_left = storm_ring;
        int c_right = MAP_WIDTH - 1 - storm_ring;

        /* top and bottom rows*/
        for(int c = c_left; c <= c_right; c++){
            add_cloud_tile(r_top, c);
            add_cloud_tile(r_bottom, c);
        }
        /*left and right cols*/
        for(int row = r_top; row <= r_bottom; row++){
            add_cloud_tile(row, c_left);
            add_cloud_tile(row, c_right);
        }

        storm_ring++;
        next_storm_tick = game_timer + STORM_TILE_INTERVAL;
    }

    /* damage players in the storm*/
    if(game_timer % 60 == 0){
        for(int i = 0; i < player_count; i++){
            Entity *p = players[i];
            if(!p->active || p->dying) continue;

            int row = (p->hitbox_y + p->hitbox_h) / TILE_H;
            int col = (p->hitbox_x + p->hitbox_w / 2) / TILE_W;
            if (obstacle_map_get(row, col) & TILE_FLAG_DAMAGE) {
                p->was_hit = 1;
                p->damage += storm_damage;
            }
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

    /*draw potions*/
    for (int i = 0; i < MAX_POTIONS; i++){ 
        //erase if flagged
        if (cur_buf == 0 && potions[i].erase_b0){
            erase_sprite(potions[i].x, potions[i].y, potions[i].sprite.width, potions[i].sprite.height);
            potions[i].erase_b0 = 0;
        }
        if (cur_buf == 1 && potions[i].erase_b1){
            erase_sprite(potions[i].x, potions[i].y, potions[i].sprite.width, potions[i].sprite.height);
            potions[i].erase_b1 = 0;
        }

        if(!potions[i].active) continue;
        //draw if active
        
        /*Inital draw into both buffers*/
        if(cur_buf == 0 && potions[i].needs_draw_b0){
            draw_sprite(&potions[i].sprite, potions[i].x, potions[i].y, 0, 0);
            potions[i].needs_draw_b0 = 0;
            potions[i].flash_drawn = 1;
        }
        if(cur_buf == 1 && potions[i].needs_draw_b1){
            draw_sprite(&potions[i].sprite, potions[i].x, potions[i].y, 0, 0);
            potions[i].needs_draw_b1 = 0;
            potions[i].flash_drawn = 1;
        }

        /*Flash logic*/
        if (potions[i].needs_draw_b0 || potions[i].needs_draw_b1) continue;

        /* Flash in its last 3s: (visble for 15 frames, hidden for 15)*/
        int should_show;
        if(potions[i].lifetime <= 180){
            should_show = (potions[i].lifetime / 15 ) % 2;
        }
        else{
            should_show = 1;
        }

        if(should_show && !potions[i].flash_drawn){
            potions[i].needs_draw_b0 = 1;
            potions[i].needs_draw_b1 = 1;
            potions[i].flash_drawn = 1;
        }
        else if(!should_show && potions[i].flash_drawn){
            potions[i].erase_b0    = 1;
            potions[i].erase_b1    = 1;
            potions[i].flash_drawn = 0;
        }
       
    }

    /*now draw storm clouds*/

    for(int i = 0; i < cloud_tile_count; i++){
        CloudTile *ct = &cloud_tiles[i];
        int px = ct->col * TILE_W;
        int py = ct->row * TILE_H;

        if(cur_buf == 0 && ct->draw_b0){
            draw_sprite(&ct->sprite, px, py, 0, 0);
            ct->draw_b0 = 0;
            continue;
        }
        if(cur_buf == 1 && ct->draw_b1){
            draw_sprite(&ct->sprite, px, py, 0, 0);
            ct->draw_b1 = 0;
            continue;
        }

        //now only redraw the tiles the player overlaps
        int player_overlap = 0;
        int tile_x0 = px;
        int tile_x1 = px + TILE_W;
        int tile_y0 = py;
        int tile_y1 = py + TILE_H;

        Entity *players[2] = {g_p1, g_p2};
        for(int j = 0; j < 2; j++){
            Entity *p = players[j];

             /* Current position */
            if (p->x < tile_x1 && p->x + PLAYER_W > tile_x0 &&
                p->y < tile_y1 && p->y + PLAYER_H > tile_y0) {
                player_overlap = 1;
                break;
            }
            /* Previous position (cuz erases restores this region) */
            if (p->prev_x[cur_buf] < tile_x1 && p->prev_x[cur_buf] + PLAYER_W > tile_x0 &&
                p->prev_y[cur_buf] < tile_y1 && p->prev_y[cur_buf] + PLAYER_H > tile_y0) {
                player_overlap = 1;
                break;
            }
        }

        if (player_overlap){
            draw_sprite(&ct->sprite, px, py, 0, 0);
        }
    }

    /* draw health bars on top of background */
    draw_health_bar(g_p1);
    draw_health_bar(g_p2);
}
