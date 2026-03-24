#ifndef SPRITE_H
#define SPRITE_H

//this will be used to determine if we should not draw that pixel
#define TRANSPARENT 0xF81F //magenta

typedef enum {
    //Player
    SPRITE_PLAYER,

    //Enemies
    SPRITE_ENEMY,

    //Projectiles
    SPRITE_PROJECTILE,

    //Background tiles
    SPRITE_TILE_GRASS,
    SPRITE_TILE_DIRT,
    SPRITE_TILE_STONE,

    SPRITE_TILE_WATER,
    SPRITE_TILE_SAND,
    SPRITE_TILE_LAVA,
    SPRITE_TILE_SNOW,
    SPRITE_TILE_WATER_BRIGHT,
    SPRITE_TILE_ICE,

    SPRITE_POSION_CLOUD,
    SPRITE_WOOD_SHIELD,
    SPRITE_LIFE_POT,
    SPRITE_COUNT
} SpriteID;

typedef struct{
    int width;
    int height;
    const short *data;
} Sprite;

extern Sprite sprites[SPRITE_COUNT];


#endif