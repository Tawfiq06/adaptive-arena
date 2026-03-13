#ifndef PLAYER_H
#define PLAYER_H

#define HEALTH 100
#define PLAYER_W 10
#define PLAYER_H 10

#include "entity.h"
#include "sprite.h"
#include "player_config.h"

void player_init(Entity *p, SpriteID sprite, short _colour, const PlayerConfig *cfg);
void player_update(Entity *p, int cur_buf);
void player_draw(const Entity *p);

#endif