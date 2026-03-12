#ifndef ENTITY_H
#define ENTITY_H

#define MAX_ENTITIES 64

typedef enum{
    ENTITY_NONE,
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_PROJECTILE
} EntityType;

typedef struct{
    int x;
    int y;

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

    int active;
} Entity;

extern Entity entities[MAX_ENTITIES];

Entity* spawn_entity(EntityType type);
void entity_update_all();
void entity_draw_all();

#endif