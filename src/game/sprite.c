#include "sprite.h"
#include "player_sprites.h"
#include "tile_sprites.h"
#include "vga.h"
#include "soldier_frames.h"
#include "projectile_sprites.h"
#include <stdlib.h>

Sprite sprites[SPRITE_COUNT] = {
   [SPRITE_PLAYER]     = {SOLDIER_W, SOLDIER_H, NULL}, //drawn using draw solider
    [SPRITE_ENEMY]      = {TILE_W, TILE_H, stone_sprite},  // placeholder
    [SPRITE_PROJECTILE] = {PROJECTILE_W, PROJECTILE_H, arrow_sprite},
    [SPRITE_TILE_GRASS] = {TILE_W,   TILE_H,   grass_sprite},
    [SPRITE_TILE_DIRT]  = {TILE_W,   TILE_H,   dirt_sprite},
    [SPRITE_TILE_STONE] = {TILE_W,   TILE_H,   stone_sprite},
    [SPRITE_TILE_WATER] = {TILE_W,   TILE_H,   water_sprite},
    [SPRITE_TILE_SAND] = {TILE_W, TILE_H, sand_sprite},
    [SPRITE_WOOD_SHIELD] = {WOOD_SHIELD_W, WOOD_SHIELD_H, wood_shield},
    [SPRITE_LIFE_POT] = {LIFE_POT_SPRITE_W, LIFE_POT_SPRITE_H, life_pot_sprite},
};