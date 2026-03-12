#include "tile_sprites.h"

static const short grass_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x03E0
};
static const short dirt_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x8400
};
static const short stone_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x7BEF
};
static const short water_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x001F
};