#ifndef PLAYER_H
#define PLAYER_H

#include "soldier_frames.h"

#define HEALTH 100
#define PLAYER_SPEED 2
#define PLAYER_HITBOX_OFFSET_X 10
#define PLAYER_HITBOX_OFFSET_Y 8
#define PLAYER_W SOLDIER_W
#define PLAYER_H SOLDIER_H
#define ATTACK_1_DAMAGE 10
#define ATTACK_2_DAMAGE 15

#include "entity.h"
#include "sprite.h"
#include "player_config.h"

void player_init(Entity *p, SpriteID sprite, short _colour, const PlayerConfig *cfg, int x_start, int flip);
void player_update(Entity *p, int cur_buf);
void player_draw(const Entity *p);

#endif