#ifndef PLAYER_H
#define PLAYER_H

#include "soldier_frames.h"

#define HEALTH 100
#define PLAYER_SPEED 2
#define PLAYER_HITBOX_OFFSET_X 7
#define PLAYER_HITBOX_OFFSET_Y 8
#define PLAYER_HITBOX_W 13
#define PLAYER_HITBOX_H 18
#define PLAYER_W SOLDIER_W
#define PLAYER_H SOLDIER_H

#define ATTACK_1_DAMAGE 10
#define ATK1_COOLDOWN 20

#define ATTACK_2_DAMAGE 18
#define ATK2_COOLDOWN 30

#define PROJECTILE_DAMAGE 12
#define SHOOT_COOLDOWN 45

#define DASH_COOLDOWN 60
#define DASH_TIMER 15

#define BLOCK_COOLDOWN 30
#define BLOCK_TIMER 15

#include "entity.h"
#include "sprite.h"
#include "player_config.h"

void player_init(Entity *p, SpriteID sprite, short _colour, const PlayerConfig *cfg, int x_start, int flip);
void player_update(Entity *p, int cur_buf);
void player_draw(const Entity *p);
void draw_health_bar(Entity* p) 
  

#endif
