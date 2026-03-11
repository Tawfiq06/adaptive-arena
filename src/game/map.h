#ifndef MAP_H
#define MAP_H

#include "sprite.h"

#define MAP_WIDTH 20
#define MAP_HEIGHT 15

void map_init(int map_index);
SpriteID map_get_tile(int row, int col);
void map_set_tile(int row, int col, SpriteID id);

#endif