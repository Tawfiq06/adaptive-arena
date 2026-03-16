#include "entity.h"
#include <stdlib.h>
#include "player.h"
#include "sprite.h"
#include "renderer.h"

Entity entities[MAX_ENTITIES];

Entity* spawn_entity(EntityType type){
    for(int i = 0; i < MAX_ENTITIES; i++){
        if(!entities[i].active){
            entities[i].active = 1;
            entities[i].type = type;
            entities[i].player_cfg = NULL; //since not player, if player use player_init
            return &entities[i];
        }
    }

    return NULL;
}

void entity_update_all(int cur_buf){
    for(int i = 0; i < MAX_ENTITIES; i++){
        if (!entities[i].active)
            continue;
        switch(entities[i].type){
            case ENTITY_PLAYER:
                player_update(&entities[i], cur_buf);
                break;
            
            case ENTITY_ENEMY:
                enemy_update(&entities[i]);
                break;
            
            case ENTITY_PROJECTILE:
                projectile_update(&entities[i]);
                break;
            default: 
                break;
        }
    }
}

void entity_draw_all()
{
    for(int i = 0; i < MAX_ENTITIES; i++)
    {
        if(entities[i].active)
            draw_entity(&entities[i]);
    }
}

void draw_entity(Entity *e){
    if (e->type == ENTITY_PLAYER){
        player_draw(e);
    }
    else{
        draw_sprite(&sprites[e->sprite_id], e->x, e->y);
    }
}

void entity_erase_all(int cur_buf){
    for (int i = 0; i < MAX_ENTITIES; i++){
        if (!entities[i].active){
            continue;
        }
        erase_sprite(entities[i].prev_x[cur_buf], entities[i].prev_y[cur_buf], entities[i].width, entities[i].height);
    }
}

