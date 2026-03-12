#include "player.h"
#include "vga.h"

void player_init(Entity *p, SpriteID sprite, short _colour){
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
}

void player_update(Entity *p){
    p->x += p->dx;
    p->y += p->dy;

    if(p->x < 0){
        p->x = 0;
    }
    if(p->y < 0){
        p->y = 0;
    }

    if(p->x + p->width > SCREEN_WIDTH)
        p->x = SCREEN_WIDTH - p->width;

    if(p->y + p->height > SCREEN_HEIGHT)
        p->y = SCREEN_HEIGHT - p->height;
}

void player_draw(const Entity *p){
    draw_rect(p->x, p->y, p->width, p->height, p->colour);
}