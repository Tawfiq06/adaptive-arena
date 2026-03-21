#ifndef SPRITE_H
#define SPRITE_H

//this will be used to determine if we should not draw that pixel
#define TRANSPARENT 0xF81F //magenta

#define SPRITE_COUNT 8
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

    SPRITE_WOOD_SHIELD
} SpriteID;

typedef struct{
    int width;
    int height;
    const short *data;
} Sprite;

extern Sprite sprites[SPRITE_COUNT];


#endif