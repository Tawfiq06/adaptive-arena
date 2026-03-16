#ifndef ENTITY_H
#define ENTITY_H

#define MAX_ENTITIES 64
#include "animator.h"
#include "player_config.h"

typedef enum{
    ENTITY_NONE,
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_PROJECTILE
} EntityType;

typedef struct{
    int x;
    int y;

    int prev_x[2];
    int prev_y[2];

    int dx;
    int dy;

    int width;
    int height;

    char facing; //this will be used to determine which direction they are facing

    int hitbox_offset_x;
    int hitbox_offset_y;
    int hitbox_w;
    int hitbox_h;

    int health;
    int sprite_id;

    short colour;
    
    EntityType type;

    Animator anim;
    const AnimDef *anim_def; //points to SOLDIER_ANIMS, PROJECTILE, etc

    const PlayerConfig *player_cfg;

    int active;
} Entity;

extern Entity entities[MAX_ENTITIES];

Entity* spawn_entity(EntityType type);
void entity_update_all(int cur_buf);
void entity_draw_all();
void entity_erase_all(int cur_buf);

#endif