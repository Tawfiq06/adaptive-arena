#include "player.h"
#include "vga.h"
#include "keyboard.h"
#include "animator.h"
#include "soldier_frames.h"

#define PLAYER_SPEED 2
#define HITBOX_OFFSET_X 10
#define HITBOX_OFFSET_Y 8
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

    p->hitbox_x = p->x + 10;
    p->hitbox_y = p->y + 8;

    p->hitbox_w = 20;
    p->hitbox_h = 18;

    p->sprite_id = (int)sprite;
    p->colour = _colour; //white

    p->type = ENTITY_PLAYER;
    p->active = 1;
    
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
    
}

void player_update(Entity *p, int cur_buf){
    int anim_finished = anim_tick(&p->anim, p->anim_def);
    
    int locked = (p->anim.anim == SOLDIER_ATK1 ||
                  p->anim.anim == SOLDIER_ATK2 ||
                  p->anim.anim == SOLDIER_HURT  ||
                  p->anim.anim == SOLDIER_DEATH);

    /* Advance animation — returns 1 if one-shot just ended */
    if (anim_finished && !locked) {
        anim_play(&p->anim, p->anim_def, SOLIDER_IDLE);
    }

    if (locked) return;

    /* Attack input (takes priority over movement) */
    if (key_pressed(p->player_cfg->key_atk1)) {
        anim_play(&p->anim, p->anim_def, SOLIDER_IDLE);
        p->attack_s1 = 1;
        return;
    }
    if (key_pressed(p->player_cfg->key_atk2)) {
        anim_play(&p->anim, p->anim_def, SOLIDER_IDLE);
        p->attack_s2 = 1;
        return;
    }

    /* Movement */
    p->dx = 0;
    p->dy = 0;

    if (key_pressed(p->player_cfg->key_up)) {
        p->dy = -PLAYER_SPEED;
       // p->facing = 'n';
    }
    if(key_pressed(p->player_cfg->key_down)){
        p->dy = PLAYER_SPEED;
      //  p->facing = 's';
    }
    if (key_pressed(p->player_cfg->key_left))  { 
        p->dx = -PLAYER_SPEED;
        p->facing = 'w'; 
        p->anim.flip = 1;
    }
    if (key_pressed(p->player_cfg->key_right)) { 
        p->dx =  PLAYER_SPEED; 
        p->facing = 'e'; 
        p->anim.flip = 0;
    }

    //Check if we should play idle animation
    int moving = (p->dx != 0 || p->dy != 0);
    anim_play(&p->anim, p->anim_def, moving ? SOLDIER_WALK : SOLIDER_IDLE);

    /*Save old postions*/
    p->prev_x[cur_buf] = p->x;
    p->prev_y[cur_buf] = p->y;

    p->x += p->dx;
    p->y += p->dy;

    /*Now clamp*/
    if (p->x < 0){
        p->x = 0;
    }                       
    if (p->y < 0){
        p->y = 0;
    }

    if(p->x + p->width > SCREEN_WIDTH)
        p->x = SCREEN_WIDTH - p->width;

    if(p->y + p->height > SCREEN_HEIGHT)
        p->y = SCREEN_HEIGHT - p->height;

     if(p->facing == 'e'){
        p->hitbox_x = p->x + HITBOX_OFFSET_X;
    }
    
    if(p->facing == 'w'){
        p->hitbox_x = p->x + SOLIDER_W - HITBOX_OFFSET_X - p->hitbox_w;
    }
}

void player_draw(const Entity *p){
    draw_soldier(&p->anim, p->x, p->y);
}
