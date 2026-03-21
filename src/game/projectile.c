#include "projectile.h"
#include "projectile_sprites.h"
#include "soldier_frames.h"
#include "renderer.h"
#include "vga.h"

#define PROJECTILE_HITBOX_OFFSET_X 0

void projectile_init(Entity *e, SpriteID sprite, int x, int y, char facing){
    e->x = x;
    e->y = y + (SOLDIER_H >> 2) - (PROJECTILE_H >> 2);

    e->width = PROJECTILE_W;
    e->height = PROJECTILE_H;

    e->hitbox_w = 4;
    e->hitbox_h = 4;

    if(facing == 'e'){
        e->dx = PROJECTILE_SPEED;
        e->hitbox_x = e->x + e->width - e->hitbox_w;
    }
    else if(facing == 'w'){
        e->dx = -PROJECTILE_SPEED;
        e->hitbox_x = e->x + PROJECTILE_HITBOX_OFFSET_X;
    }

    //current y + half of height - half of hitbox height, gives hitbox y
    e->hitbox_y = e->y + (e->height >> 2) - (e->hitbox_h >> 2);
    
    e->facing = facing;
    e->sprite_id = (int) sprite;

    e->type = ENTITY_PROJECTILE;
    e->active = 1;

    e->prev_x[0] = e->x;
    e->prev_x[1] = e->x;
    e->prev_y[0] = e->y;
    e->prev_y[1] = e->y;
}

void projectile_update(Entity *e, int cur_buf){
    e->prev_x[cur_buf] = e->x - e->dx;
    e->prev_y[cur_buf] = e->y - e->dy;

    e->x += e->dx;
    e->y += e->dy;

    if(e->facing == 'e'){
        e->hitbox_x = e->x + e->width - e->hitbox_w;
    }
    else if(e->facing == 'w'){
        e->hitbox_x = e->x + PROJECTILE_HITBOX_OFFSET_X;
    }

    e->hitbox_y = e->y + (e->height >> 2) - (e->hitbox_h >> 2);

    //make projectile vanish if out of bounds
    if(e->hitbox_x < 0 || e->hitbox_x + e->hitbox_w > SCREEN_WIDTH ||
       e->hitbox_y < 0 || e->hitbox_y + e->hitbox_h > SCREEN_HEIGHT){
        e->pending_erase = 1;
        e->pending_erase_b1 = 1;
        e->pending_erase_b2 = 1;
    }
}

void projectile_draw(Entity *e){
    int flip_h = 0;
    if(e->facing == 'w'){
        flip_h = 1;
    }
    draw_sprite(&sprites[e->sprite_id], e->x, e->y, flip_h, 0);
}