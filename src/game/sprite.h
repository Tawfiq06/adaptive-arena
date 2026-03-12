#ifndef SPRITE_H
#define SPRITE_H

//this will be used to determine if we should not draw that pixel
#define TRANSPARENT 0xF81F //magenta

#define SPRITE_COUNT 7
typedef enum {
    //Player
    SPRITE_PLAYER = 0,

    //Enemies
    SPRITE_ENEMY = 1,

    //Projectiles
    SPRITE_PROJECTILE = 2,

    //Background tiles
    SPRITE_TILE_GRASS = 3,
    SPRITE_TILE_DIRT  = 4,
    SPRITE_TILE_STONE = 5,
    SPRITE_TILE_WATER = 6
} SpriteID;

typedef struct{
    int width;
    int height;
    const short *data;
} Sprite;

extern Sprite sprites[SPRITE_COUNT];


#endif