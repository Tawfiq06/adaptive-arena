#include "entity.h"
#include <stdlib.h>
#include "player.h"
#include "sprite.h"

Entity entities[MAX_ENTITIES];

Entity* spawn_entity(EntityType type){
    for(int i = 0; i < MAX_ENTITIES; i++){
        if(!entities[i].active){
            entities[i].active = 1;
            entities[i].type = type;
            return &entities[i];
        }
    }

    return NULL;
}

void entity_update_all(){
    for(int i = 0; i < MAX_ENTITIES; i++){
        if (!entities[i].active)
            continue;
        switch(entities[i].type){
            case ENTITY_PLAYER:
                player_update(&entities[i]);
                break;
            
            case ENTITY_ENEMY:
                enemy_update(&entities[i]);
                break;
            
            case ENTITY_PROJECTILE:
                projectile_update(&entities[i]);
                break;
        }
    }
}

void entity_draw_all()
{
    for(int i = 0; i < MAX_ENTITIES; i++)
    {
        if(!entities[i].active)
            continue;

        draw_entity(&entities[i]);
    }
}

void draw_entity(Entity *e){
    draw_sprite(&sprites[e->sprite_id], e->x, e->y);
}