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
    int is_ai;
    
    int x;
    int y;

    int prev_x[2];
    int prev_y[2];

    int dx;
    int dy;

    int width;
    int height;

    char facing; //this will be used to determine which direction they are facing

    int hitbox_x;
    int hitbox_y;
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

    //flags to know if certain actions need to happen
    int attack_s1;
    int attack_s2;
    int attack_p;

    //hit flag
    int was_hit;
    int damage;

    /* Set when dying: keep drawing death anim until it finishes*/
    int dying;

    int pending_erase;
    int pending_erase_b1; //to check both buffers for pending
    int pending_erase_b2;

    /* cooldowns (number of frames) */
    int atk1_cooldown;
    int atk2_cooldown;

    int shoot_cooldown;
    int arrow_fired;

    int dash_cooldown;
    int dash_timer;
    int is_dashing; //use to check if speed needs to be *2

    int block_cooldown;
    int block_timer;
    int blocking;

    struct Entity* owner;

    int on_ice;
    int ice_dx;
    int ice_dy;
    int store_dx;
    int store_dy;
} Entity;

extern Entity entities[MAX_ENTITIES];

Entity* spawn_entity(EntityType type);
void entity_update_all(int cur_buf);
void entity_draw_all();
void entity_erase_all(int cur_buf);
void draw_entity(Entity *e);

#endif
