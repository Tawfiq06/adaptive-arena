#include "player.h"
#include "vga.h"
#include "keyboard.h"
#include "animator.h"
#include "renderer.h"
#include "map.h"
#include "obstacle_map.h"
#include "decorations.h"
#include "tile_sprites.h"
#include "audio.h"
#include "attack_1_sword_swing_1.h"

void player_init(Entity *p, SpriteID sprite, short _colour, const PlayerConfig *cfg, int start_x, int flip){
    p->player_cfg = cfg; //not needed at init
    p->x = start_x;
    p->y = SCREEN_HEIGHT / 2 - PLAYER_H / 2;

    p->width = PLAYER_W;
    p->height = PLAYER_H;

    p->dx = 0;
    p->dy = 0;

    p->health = HEALTH;

    p->facing = flip ? 'w' : 'e';  //n for north (up), s for south(dowm), e for east(right), w for west (left)

    p->hitbox_x = p->x + PLAYER_HITBOX_OFFSET_X;
    p->hitbox_y = p->y + PLAYER_HITBOX_OFFSET_Y;

    p->hitbox_w = PLAYER_HITBOX_W;
    p->hitbox_h = PLAYER_HITBOX_H;

    p->sprite_id = (int)sprite;
    p->colour = _colour; //white

    p->type = ENTITY_PLAYER;
    p->active = 1;
    p->dying = 0;
    
    p->anim_def = SOLDIER_ANIMS;

    anim_init(&p->anim, p->anim_def, SOLIDER_IDLE);
    p->anim.flip = flip;
   
    p->prev_x[0] = p->x;
    p->prev_x[1] = p->x;
    p->prev_y[0] = p->y;
    p->prev_y[1] = p->y;

    p->attack_s1 = 0;
    p->attack_s2 = 0;
    p->attack_p = 0;

    p->was_hit = 0;
    p->damage = 0;

    p->pending_erase = 0;
    p->shoot_cooldown = 0;
    p->arrow_fired = 0;

    p->atk1_cooldown = 0;
    p->atk2_cooldown = 0;

    p->dash_cooldown = 0;
    p->dash_timer = 0;
    p->is_dashing = 0;

    p->block_cooldown = 0;
    p->block_timer = 0;
    p->blocking = 0;
}

void player_update(Entity *p, int cur_buf){
    /*Save postion before returning so erase works*/
    p->prev_x[cur_buf] = p->x;
    p->prev_y[cur_buf] = p->y;
    
    /* 1. Tick animation first - returns 1 when one shot is done*/
    int anim_finished = anim_tick(&p->anim, p->anim_def);
    
    /* 2. If currently play death animation, wait for it to finish*/
    if (p->dying){
        if(anim_finished){
            p->active = 0; /*deactive after death anim*/
        }
        return;
    }

    /* 3. Handle incoming hit */
    if(p->was_hit){
        p->was_hit = 0;
        p->health -= p->damage;
        p->damage = 0;

        if(p->health < 0){
            p->health = 0;
            p->dying = 1;
            anim_play(&p->anim, p->anim_def, SOLDIER_DEATH);
        }
        else{
            anim_play(&p->anim, p->anim_def, SOLDIER_HURT);
        }
        return;
    }

    /* 4. Locked if you shouldnt be able to move during the animation*/
    int locked = (p->anim.anim == SOLDIER_ATK1 ||
                  p->anim.anim == SOLDIER_ATK2 ||
                  //p->anim.anim == SOLDIER_ATK3 ||
                  p->anim.anim == SOLDIER_HURT  ||
                  p->anim.anim == SOLDIER_DEATH);

    /* Advance animation — returns 1 if one-shot just ended */
    if (anim_finished) {
        p->attack_s1 = 0;
        p->attack_s2 = 0;
        anim_play(&p->anim, p->anim_def, SOLIDER_IDLE);
        locked = 0;
    }
    
    /*decect release frame for arrow*/
    if (p->anim.anim == SOLDIER_ATK3 && p->anim.frame == SOLDIER_ANIMS[SOLDIER_ATK3].end - 1 && !p->attack_p && !p->arrow_fired){
        p->attack_p = 1;
        p->arrow_fired = 1;
    }

    if (locked) return;

    /* Handle cooldowns*/
    if(p->atk1_cooldown > 0){
        p->atk1_cooldown--;
    }

    if(p->atk2_cooldown > 0){
        p->atk2_cooldown--;
    }

    if(p->shoot_cooldown > 0){
        p->shoot_cooldown--;
    }

    if(p->dash_cooldown > 0 && !p->is_dashing){
        p->dash_cooldown--;
    }

    if(p->block_cooldown > 0 && !p->blocking){
        p->block_cooldown--;
    }

    /* Attack input (takes priority over movement) */
    const PlayerConfig *cfg = p->player_cfg;
    if (key_pressed(cfg->key_atk1) && p->atk1_cooldown == 0 && p->anim.anim != SOLDIER_ATK1) {
        anim_play(&p->anim, p->anim_def, SOLDIER_ATK1);
        p->atk1_cooldown = ATK1_COOLDOWN;
        p->attack_s1 = 1;
        //play_sfx(attack_1_sword_swing_1, ATTACK_1_SWORD_SWING_1_LENGTH, 1.0f, 0);
        return;
    }
    if (key_pressed(cfg->key_atk2) && p->atk2_cooldown == 0 && p->anim.anim != SOLDIER_ATK2) {
        anim_play(&p->anim, p->anim_def, SOLDIER_ATK2);
        p->atk2_cooldown = ATK2_COOLDOWN;
        p->attack_s2 = 1;
        return;
    }

    if (key_pressed(cfg->key_atkp) && p->shoot_cooldown == 0 && p->anim.anim != SOLDIER_ATK3){
        anim_play(&p->anim, p->anim_def, SOLDIER_ATK3);
        p->shoot_cooldown = SHOOT_COOLDOWN;
        p->arrow_fired = 0;
        p->attack_p = 0;
        return;
    }

    if(key_pressed(cfg->key_dash) && p->dash_cooldown == 0){
        p->dash_cooldown = DASH_COOLDOWN;
        p->is_dashing = 1;
        p->dash_timer = DASH_TIMER;
    }

    if(key_pressed(cfg->key_block) && p->block_cooldown == 0){
        p->block_cooldown = BLOCK_COOLDOWN;
        p->blocking = 1;
        p->block_timer = BLOCK_TIMER;
    }

    /* Movement */
    if(!p->is_dashing && !p->on_ice){
        p->dx = 0;
        p->dy = 0;
    }

    int pressed_key = 0; 
    if (!p->is_dashing && key_pressed(p->player_cfg->key_up)) {
        if(!p->on_ice || (p->store_dx == 0 && p->store_dy == 0) || p->ice_stored_reset){
            p->dy = -PLAYER_SPEED;
        }
        pressed_key = 1;
       // p->facing = 'n';
    }
    if(!p->is_dashing && key_pressed(p->player_cfg->key_down)){
        if(!p->on_ice || (p->store_dx == 0 && p->store_dy == 0) || p->ice_stored_reset){
            p->dy = PLAYER_SPEED;
        }
        pressed_key = 1;
      //  p->facing = 's';
    }
    if (!p->is_dashing && key_pressed(p->player_cfg->key_left))  { 
        if(!p->on_ice || (p->store_dx == 0 && p->store_dy == 0) || p->ice_stored_reset){
            p->dx = -PLAYER_SPEED;
        }
        p->facing = 'w'; 
        p->anim.flip = 1;
        pressed_key = 1;
    }
    if (!p->is_dashing && key_pressed(p->player_cfg->key_right)) { 
        if(!p->on_ice || (p->store_dx == 0 && p->store_dy == 0) || p->ice_stored_reset){
            p->dx =  PLAYER_SPEED; 
        }
        p->facing = 'e'; 
        p->anim.flip = 0;
        pressed_key = 1;
    }

    //half player movement while bow is drawn
    if(p->anim.anim == SOLDIER_ATK3 && !p->arrow_fired){
        p->dx = p->dx >> 1;
        p->dy = p->dy >> 1;
    }
    if(p->anim.anim == SOLDIER_ATK3 && p->arrow_fired){
        p->dx = p->dx << 1;
        p->dy = p->dy << 1;
    }

    if(p->is_dashing && p->dash_timer == DASH_TIMER){
        p->dx = p->dx << 1;
        p->dy = p->dy << 1;
    }
    if(p->is_dashing){
        p->dash_timer--;
        if(p->dash_timer == 0){
            p->is_dashing = 0;
            p->dx = p->dx >> 1;
            p->dy = p->dy >> 1;
        }
    }

    if(p->blocking){
        p->dx = p->dx >> 1;
        p->dy = p->dy >> 1;
        p->block_timer--;
    }
    if(p->blocking && p->block_timer == 0){
        p->dx = p->dx << 1;
        p->dy = p->dy << 1;
        p->blocking = 0;
    }

    //check for colisions with decor
    int feet_y  = p->hitbox_y + p->hitbox_h;
    int mid_x   = p->hitbox_x + (p->hitbox_w >> 1);

    if (p->dx != 0) {
    int nx = (p->dx > 0) ? p->hitbox_x + p->hitbox_w + p->dx : p->hitbox_x + p->dx;
    int row = (p->hitbox_y + p->hitbox_h - 1) >> 4;
    int col = nx >> 4;

        if (obstacle_map_get(row, col) & TILE_FLAG_SOLID) {
            /* Tile is flagged, now check against decorations in that cell */
            const DecoCell *cell = deco_map_get_cell(row, col);
            int blocked = 0;
            for (int i = 0; i < cell->count; i++) {
                int idx = cell->indices[i];
                if (!deco_is_solid(decorations[idx].type)) continue;
                const DecoType *dt = &DECO_LOOKUP[decorations[idx].type];

                /* Base rect only — bottom 8px of the decoration */
                int dx = decorations[idx].x;
                int dy = decorations[idx].y + dt->h - 8;
                int dw = dt->w;
                int dh = 8;

                int pl = p->hitbox_x + p->dx;
                int pr = pl + p->hitbox_w - 1;
                int pt = p->hitbox_y;
                int pb = p->hitbox_y + p->hitbox_h - 1;

                if (pr >= dx && pl <= dx + dw - 1 &&
                    pb >= dy && pt <= dy + dh - 1) {
                    blocked = 1;
                    break;
                }
            }
            if (blocked){
                p->dx = 0;
                p->ice_stored_reset = 1;
            } 
        }
    }

    if (p->dy != 0) {
        int ny = (p->dy > 0) ? p->hitbox_y + p->hitbox_h + p->dy : p->hitbox_y + p->dy;
        int row = ny >> 4;
        int col = (p->hitbox_x + p->hitbox_w / 2) >> 4;

        if (obstacle_map_get(row, col) & TILE_FLAG_SOLID) {
            const DecoCell *cell = deco_map_get_cell(row, col);
            int blocked = 0;
            for (int i = 0; i < cell->count; i++) {
                int idx = cell->indices[i];
                if (!deco_is_solid(decorations[idx].type)) continue;
                const DecoType *dt = &DECO_LOOKUP[decorations[idx].type];

                int dx = decorations[idx].x;
                int dy = decorations[idx].y + dt->h - 8;
                int dw = dt->w;
                int dh = 8;

                int pl = p->hitbox_x;
                int pr = pl + p->hitbox_w - 1;
                int pt = p->hitbox_y + p->dy;
                int pb = pt + p->hitbox_h - 1;

                if (pr >= dx && pl <= dx + dw - 1 &&
                    pb >= dy && pt <= dy + dh - 1) {
                    blocked = 1;
                    break;
                }
            }
            if (blocked){ 
                p->dy = 0;
                p->ice_stored_reset = 1;
            }
        }
    }

    int on_ice_now = obstacle_map_at_pixel(mid_x, feet_y) & TILE_FLAG_ICE;
    if(on_ice_now){
        if(!p->on_ice || (p->ice_dx == 0 && p->ice_dy == 0) || p->ice_stored_reset){
            //just got on the ice
            p->ice_dx = p->dx;
            p->ice_dy = p->dy;
            p->store_dx = p->dx;
            p->store_dy = p->dy;
            p->ice_stored_reset = 0;
        }

        if(pressed_key){
            p->ice_dx = p->store_dx;
            p->ice_dy = p->store_dy;
        }

        p->on_ice = 1;

        //ignore input use stored veloctiy
        p->dx = p->ice_dx;
        p->dy = p->ice_dy;

        //add some friction
        p->ice_dx = (p->ice_dx * 7) / 8;
        p->ice_dy = (p->ice_dy * 7) / 8;

        if (p->ice_dx > -1 && p->ice_dx < 1) p->ice_dx = 0;
        if (p->ice_dy > -1 && p->ice_dy < 1) p->ice_dy = 0;
    }
    else{
        p->on_ice = 0;
    }

    // Slow tiles
    if (!p->is_dashing && obstacle_map_at_pixel(mid_x + p->dx, feet_y + p->dy) & TILE_FLAG_SLOW) {
        p->dx /= 2;
        p->dy /= 2;
    }

    // --- Animation ---
    if (p->anim.anim != SOLDIER_ATK3) {
        int moving = (p->dx != 0 || p->dy != 0);
        anim_play(&p->anim, p->anim_def, moving ? SOLDIER_WALK : SOLIDER_IDLE);
    }

    // --- Move hitbox, clamp hitbox to screen ---
    p->hitbox_x += p->dx;
    p->hitbox_y += p->dy;

    if (p->hitbox_x < 0){
        p->hitbox_x = 0;
        p->ice_stored_reset = 1;
    }
    if (p->hitbox_x + p->hitbox_w > SCREEN_WIDTH){
        p->hitbox_x = SCREEN_WIDTH - p->hitbox_w;
        p->ice_stored_reset = 1;
    }
    if (p->hitbox_y < 0){
        p->hitbox_y = 0;
        p->ice_stored_reset = 1;
    }
    if (p->hitbox_y + p->hitbox_h > SCREEN_HEIGHT){
        p->hitbox_y = SCREEN_HEIGHT - p->hitbox_h;
        p->ice_stored_reset = 1;
    }
    // --- Derive p->x / p->y from hitbox (single source of truth) ---
    p->x = p->hitbox_x - PLAYER_HITBOX_OFFSET_X;
    p->y = p->hitbox_y - PLAYER_HITBOX_OFFSET_Y;
}

void player_draw(const Entity *p){
    int draw_x = p->x;
    if(p->facing == 'w'){
        draw_x = p->x - (SOLDIER_W - 2 * PLAYER_HITBOX_OFFSET_X - PLAYER_HITBOX_W);
    }
    draw_soldier(&p->anim, draw_x, p->y);
    if(p->blocking){
        int flip_h = 0;
        int x_off = 0;
        if(p->facing == 'w'){
            flip_h = 1;
            x_off = -1;
        }
        draw_sprite(&sprites[SPRITE_WOOD_SHIELD], p->hitbox_x + x_off, p->hitbox_y + 2, flip_h, 0);
    }
    /*Use this to show player hitboxes*/
    //draw_rect_outline(p->hitbox_x, p->hitbox_y, p->hitbox_w, p->hitbox_h, 0x0000);
}

void draw_health_bar(Entity* p) {   //call this function twice, pass in each player 
    int health = p->health; // get current health 
    int health_bar = (health * 30) / 100; // divide so that 100 health = 30 pixels 
    int full_health = 30;
    int x_coord = p->player_cfg->health_x;
    int y_coord = p->player_cfg->health_y;

    draw_rect(x_coord, y_coord, full_health, 6, 0); // draw black background for health bar 
    draw_rect(x_coord, y_coord, health_bar, 6, 0xF800); // width is scaled version of health 
    draw_rect_outline(x_coord, y_coord, full_health, 6, 0);   // black outline on top of red 
}
