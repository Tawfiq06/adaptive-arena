#include "player.h"
#include "vga.h"
void player_init(Player *p, short _colour){
    p->x = 160;
    p->y = 240;

    p->width = 10;
    p->height = 10;

    p->dx = 0;
    p->dy = 0;

    p->health = HEALTH;

    p->colour = _colour;
}

void player_update(Player *p){
    p->x += p->dx;
    p->y += p->dy;
}

void player_draw(const Player *p){
    draw_rect(p->x, p->y, p->width, p->height, p->colour);
}