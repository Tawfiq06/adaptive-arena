#include "obstacle_map.h"
#include "map.h"
#include "decorations.h"
#include "sprite.h"
#include "tile_sprites.h"

unsigned char obstacle_map[MAP_HEIGHT][MAP_WIDTH];

/* Which decoration types block movement / projectiles */
static int deco_is_solid(int deco_type) {
    switch (deco_type) {
        case DECO_ROCK_BIG_BROWN:
        case DECO_ROCK_BIG_GREY:
        case DECO_ROCK_MED_BROWN:
        case DECO_ROCK_MED_GREY:
        case DECO_TREE_GREEN_A:
        case DECO_TREE_GREEN_B:
        case DECO_TREE_AUTUMN_A:
        case DECO_TREE_AUTUMN_B:
        case DECO_STICK_TREE:
            return 1;
        default:
            return 0;
    }
}

static int deco_blocks_proj(int deco_type) {
    /* Everything solid also blocks projectiles,
       plus bushes (cover but not impassable) */
    if (deco_is_solid(deco_type)) return 1;
    switch (deco_type) {
        case DECO_BUSH_GREEN_SM:
        case DECO_BUSH_OLIVE_SM:
        case DECO_BUSH_RED_SM:
        case DECO_BUSH_GREEN_LG:
        case DECO_BUSH_OLIVE_LG:
            return 1;
        default:
            return 0;
    }
}

void obstacle_map_init(void) {
    /* 1. Clear everything */
    for (int r = 0; r < MAP_HEIGHT; r++)
        for (int c = 0; c < MAP_WIDTH; c++)
            obstacle_map[r][c] = 0;

    /* 2. Stamp tile-based flags (water = slow + no projectiles) */
    for (int r = 0; r < MAP_HEIGHT; r++) {
        for (int c = 0; c < MAP_WIDTH; c++) {
            SpriteID tile = map_get_tile(r, c);
            if (tile == SPRITE_TILE_WATER) {
                obstacle_map[r][c] |= TILE_FLAG_SLOW | TILE_FLAG_NO_PROJ;
            }
        }
    }

    /* 3. Stamp decoration-based flags using the existing deco_map */
    for (int r = 0; r < MAP_HEIGHT; r++) {
        for (int c = 0; c < MAP_WIDTH; c++) {
            const DecoCell *cell = deco_map_get_cell(r, c);
            for (int i = 0; i < cell->count; i++) {
                int dtype = decorations[cell->indices[i]].type;
                if (deco_is_solid(dtype))
                    obstacle_map[r][c] |= TILE_FLAG_SOLID | TILE_FLAG_NO_PROJ;
                else if (deco_blocks_proj(dtype))
                    obstacle_map[r][c] |= TILE_FLAG_NO_PROJ;
            }
        }
    }
}

void obstacle_map_set(int row, int col, unsigned char flags) {
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) return;
    obstacle_map[row][col] |= flags;
}

void obstacle_map_clear(int row, int col, unsigned char flags) {
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) return;
    obstacle_map[row][col] &= ~flags;
}

unsigned char obstacle_map_get(int row, int col) {
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH) return 0;
    return obstacle_map[row][col];
}

int pixel_to_col(int px) { return px / TILE_W; }
int pixel_to_row(int py) { return py / TILE_H; }

unsigned char obstacle_map_at_pixel(int px, int py) {
    return obstacle_map_get(pixel_to_row(py), pixel_to_col(px));
}