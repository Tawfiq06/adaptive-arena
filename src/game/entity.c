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
        erase_sptire(entities[i].prev_x[cur_buf], entities[i].prev_y[cur_buf], entities[i].width, entities[i].height);
    }
}

static void erase_sprite(int x, int y, int w, int h){
    //convert pixel rect to tile indices
    int col0 = x >> 4;
    int col1 = (x + w + 15) >> 4;
    int row0 = y >> 4;
    int row1 = (y + h + 15) >> 4;

    for(int row = row0; row < row1; row++){
        for (int col = col0; col < col1; col++){
            SpriteID id = map_get_tile(row, col);
            draw_sprite(&sprites[id], col << 4, row << 4);
        }
    }
}