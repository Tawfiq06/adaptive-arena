#include "projectile.h"
#include "vga.h"

#define PROJECTILE_HITBOX_OFFSET_X 0

void projectile_init(Entity *e, SpriteID sprite, int x, int y, char facing){
    e->x = x;
    e->y = y;

    e->width = PROJECTILE_W;
    e->height = PROJECTILE_H;

    e->hitbox_w = 4;
    e->hitbox_h = 4;

    if(facing == 'e'){
        e->dx = PROJECTILE_SPEED;
        e->hitbox_x = e->width - e->hitbox_w;
    }
    else if(facing == 'w'){
        e->dx = -PROJECTILE_SPEED;
        e->hitbox_x = e->x + PROJECTILE_HITBOX_OFFSET_X;
    }

    //current y + half of height - half of hitbox height, gives hitbox y
    e->hitbox_y = e->y + (e->height >> 2) - (e->hitbox_h >> 2);
    
    e->facing = facing;
    e->sprite_id = sprite;

    e->type = ENTITY_PROJECTILE;
    e->active = 1;

    e->prev_x[0] = e->x;
    e->prev_x[1] = e->x;
    e->prev_y[0] = e->y;
    e->prev_y[1] = e->y;
}

void projectile_update(Entity *e, int cur_buf){
    e->prev_x[cur_buf] = e->x;
    e->prev_y[cur_buf] = e->y;

    e->x += e->dx;
    e->y += e->dy;

    //make projectile vanish if out of bounds
    if(e->hitbox_x < 0 || e->hitbox_x + e->hitbox_w > SCREEN_WIDTH ||
       e->hitbox_y < 0 || e->hitbox_y + e->hitbox_h > SCREEN_HEIGHT){
        e->active = 0;
    }
}

void projectile_draw(Entity *e){
    (void)e;
}