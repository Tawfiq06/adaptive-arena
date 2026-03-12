#ifndef PLAYER_H
#define PLAYER_H

#define HEALTH 100
#define PLAYER_W 10
#define PLAYER_H 10

#include "entity.h"
#include "sprite.h"
void player_init(Entity *p, SpriteID sprite, short _colour);
void player_update(Entity *p);
void player_draw(const Entity *p);

#endif