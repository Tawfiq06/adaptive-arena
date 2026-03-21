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
        case DECO_AUTUMN_TREE_RED_MED:
        case DECO_AUTUMN_TREE_YELLOW_MED:
        case DECO_STICK_TREE_MED:
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
                obstacle_map[r][c] |= TILE_FLAG_SLOW;
            }
        }
    }

    /* 3. Stamp decoration-based flags — bottom row only for SOLID */
    for (int i = 0; i < deco_count; i++) {
        const Decoration *d   = &decorations[i];
        const DecoType   *dt  = &DECO_LOOKUP[d->type];

        /* Bottom row only — player walks "behind" the upper part */
        int base_row  = (d->y + dt->h - 1) / TILE_H;
        int col_left  = d->x / TILE_W;
        int col_right = (d->x + dt->w - 1) / TILE_W;

        if (deco_is_solid(d->type)) {
            for (int c = col_left; c <= col_right; c++)
                obstacle_map_set(base_row, c, TILE_FLAG_SOLID);
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