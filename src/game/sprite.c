#include "sprite.h"
#include "player_sprites.h"
#include "tile_sprites.h"
#include "vga.h"
#include "soldier_frames.h"
#include <stdlib.h>

Sprite sprites[SPRITE_COUNT] = {
   [SPRITE_PLAYER]     = {SOLDIER_W, SOLDIER_H, NULL}, //drawn using draw solider
    [SPRITE_ENEMY]      = {TILE_W, TILE_H, stone_sprite},  // placeholder
    [SPRITE_PROJECTILE] = {4,        4,        player_sprite},  // placeholder
    [SPRITE_TILE_GRASS] = {TILE_W,   TILE_H,   grass_sprite},
    [SPRITE_TILE_DIRT]  = {TILE_W,   TILE_H,   dirt_sprite},
    [SPRITE_TILE_STONE] = {TILE_W,   TILE_H,   stone_sprite},
    [SPRITE_TILE_WATER] = {TILE_W,   TILE_H,   water_sprite},
};