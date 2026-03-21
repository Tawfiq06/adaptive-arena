#include "player.h"
#include "vga.h"
#include "keyboard.h"
#include "animator.h"

#define SHOOT_COOLDOWN 30

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

    p->hitbox_w = 20;
    p->hitbox_h = 18;

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
                  p->anim.anim == SOLDIER_ATK3 ||
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

    /* Attack input (takes priority over movement) */
    const PlayerConfig *cfg = p->player_cfg;
    if (key_pressed(cfg->key_atk1)) {
        anim_play(&p->anim, p->anim_def, SOLDIER_ATK1);
        p->attack_s1 = 1;
        return;
    }
    if (key_pressed(cfg->key_atk2)) {
        anim_play(&p->anim, p->anim_def, SOLDIER_ATK2);
        p->attack_s2 = 1;
        return;
    }

    if(p->shoot_cooldown > 0){
        p->shoot_cooldown--;
    }

    if (key_pressed(cfg->key_atkp) && p->shoot_cooldown == 0){
        anim_play(&p->anim, p->anim_def, SOLDIER_ATK3);
        p->shoot_cooldown = SHOOT_COOLDOWN;
        p->arrow_fired = 0;
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

    p->x += p->dx;
    p->y += p->dy;

    /*Now clamp*/
    if (p->x  < 0){
        p->x = 0;
    }                       
    if (p->y < 0){
        p->y = 0;
    }

    if(p->x + p->width > SCREEN_WIDTH)
        p->x = SCREEN_WIDTH - p->width;

    if(p->y + p->height > SCREEN_HEIGHT)
        p->y = SCREEN_HEIGHT - p->height;

    
    /* Update hitbox to follow player postion */
    if(p->facing == 'e'){
        p->hitbox_x = p->x + PLAYER_HITBOX_OFFSET_X;
    }
    
    if(p->facing == 'w'){
        p->hitbox_x = p->x + SOLDIER_W - PLAYER_HITBOX_OFFSET_X - p->hitbox_w;
    }
    p->hitbox_y = p->y + PLAYER_HITBOX_OFFSET_Y;
}

void player_draw(const Entity *p){
    draw_soldier(&p->anim, p->x, p->y);
}
