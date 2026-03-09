#ifndef PLAYER_H
#define PLAYER_H

#define HEALTH 100

typedef struct{
    int x;
    int y;
    int width;
    int height;
    short colour;
    int dx;
    int dy;
    int health;
} Player;

void player_init(Player *p, short _colour);
void player_update(Player *p);
void player_draw(const Player *p);

#endif