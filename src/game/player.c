#include "player.h"
#include "vga.h"
#include "keyboard.h"
#include "player_anim.h"

#define PLAYER_SPEED 2

void player_init(Entity *p, SpriteID sprite, short _colour, const PlayerConfig *cfg){
    p->player_cfg = cfg; //not needed at init
    p->x = SCREEN_WIDTH / 2;
    p->y = SCREEN_HEIGHT / 2;

    p->width = PLAYER_W;
    p->height = PLAYER_H;

    p->dx = 0;
    p->dy = 0;

    p->health = HEALTH;

    p->facing = 'n';  //n for north (up), s for south(dowm), e for east(right), w for west (left)

    p->hitbox_offset_x = 0;
    p->hitbox_offset_y = 0;

    p->hitbox_w = p->width;
    p->hitbox_h = p->height;

    p->sprite_id = (int)sprite;
    p->colour = _colour; //white

    p->type = ENTITY_PLAYER;
    p->active = 1;
    
    anim_init(&p->anim);

    p->prev_x[0] = p->x;
    p->prev_x[1] = p->x;
    p->prev_y[0] = p->y;
    p->prev_y[1] = p->y;
}

void player_update(Entity *p, int cur_buf){
    int anim_finished = anim_tick(&p->anim);

    if (anim_finished){
        anim_play(&p->anim, ANIM_IDLE);
    }
    
    int locked = (p->anim.anim == ANIM_ATK1 ||
                  p->anim.anim == ANIM_ATK2 ||
                  p->anim.anim == ANIM_HURT  ||
                  p->anim.anim == ANIM_DEATH);

    /* Advance animation — returns 1 if one-shot just ended */
    if (anim_tick(&p->anim) && !locked) {
        anim_play(&p->anim, ANIM_IDLE);
    }

    if (locked) return;

    /* Attack input (takes priority over movement) */
    if (key_pressed(p->player_cfg->key_atk1)) {
        anim_play(&p->anim, ANIM_ATK1);
        return;
    }
    if (key_pressed(p->player_cfg->key_atk2)) {
        anim_play(&p->anim, ANIM_ATK2);
        return;
    }

    /* Movement */
    p->dx = 0;
    p->dy = 0;

    if (key_pressed(p->player_cfg->key_up)) {
        p->dy = -PLAYER_SPEED;
        p->facing = 'n';
    }
    if(key_pressed(p->player_cfg->key_down)){
        p->dy = PLAYER_SPEED;
        p->facing = 's';
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
    anim_play(&p->anim, moving ? ANIM_WALK : ANIM_IDLE);

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
}

void player_draw(const Entity *p){
    draw_soldier(&p->anim, p->x, p->y);
}