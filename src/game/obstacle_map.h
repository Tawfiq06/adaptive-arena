#ifndef OBSTACLE_MAP_H
#define OBSTACLE_MAP_H

#include "map.h"  // for MAP_WIDTH, MAP_HEIGHT, TILE_W, TILE_H

/* Bit flags, combine with | when setting, test with & */
#define TILE_FLAG_SOLID    (1 << 0)  // blocks movement entirely */
#define TILE_FLAG_SLOW     (1 << 1)  //water halves speed */
#define TILE_FLAG_DAMAGE   (1 << 2)  // storm zone — drains HP */
#define TILE_FLAG_NO_PROJ  (1 << 3)  // kills projectiles */

extern unsigned char obstacle_map[MAP_HEIGHT][MAP_WIDTH];

void obstacle_map_init(void);

void obstacle_map_set(int row, int col, unsigned char flags);
void obstacle_map_clear(int row, int col, unsigned char flags);
unsigned char obstacle_map_get(int row, int col);

int pixel_to_col(int px);
int pixel_to_row(int py);

unsigned char obstacle_map_at_pixel(int px, int py);

#endif