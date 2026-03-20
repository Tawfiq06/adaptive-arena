#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "entity.h"

#define PROJECTILE_SPEED 4
#define PROJECTILE_W 8
#define PROJECTILE_H 5

void projectile_update(Entity *e, int cur_buf);
void projectile_draw(Entity *e);
void projectile_init(Entity *e, SpriteID sprite, int x, int y, char facing);
#endif