#include "sprite.h"
#include "player_sprites.h"
#include "vga.h"

Sprite sprites[SPRITE_COUNT] = {
    [SPRITE_PLAYER] = {PLAYER_W, PLAYER_H, player_sprite},
};