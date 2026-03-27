#include "entity.h"
#include <stdlib.h>
#include "player.h"
#include "sprite.h"
#include "renderer.h"
#include "decorations.h"
#include "projectile.h"

Entity entities[MAX_ENTITIES];

Entity* spawn_entity(EntityType type){
    for(int i = 0; i < MAX_ENTITIES; i++){
        if(!entities[i].active){
            entities[i].is_ai = 0;
            entities[i].active = 1;
            entities[i].type = type;
            entities[i].player_cfg = NULL; //since not player, if player use player_init
            entities[i].anim_def = NULL;
            entities[i].dying = 0;
            entities[i].was_hit = 0;
            entities[i].damage = 0;
            entities[i].attack_s1 = 0;
            entities[i].attack_s2 = 0;
            entities[i].attack_p = 0;
            entities[i].pending_erase_b1 = 0;
            entities[i].pending_erase_b2 = 0;
            entities[i].shoot_cooldown = 0;
            entities[i].arrow_fired = 0;
            entities[i].owner = NULL;

            entities[i].atk1_cooldown = 0;
            entities[i].atk2_cooldown = 0;

            entities[i].blocking = 0;
            entities[i].block_cooldown = 0;
            entities[i].block_timer = 0;

            entities[i].dash_cooldown = 0;
            entities[i].dash_timer = 0;
            entities[i].is_dashing = 0;
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
                projectile_update(&entities[i], cur_buf);
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
    switch (e->type){
        case ENTITY_PLAYER: 
            player_draw(e);
            break;
        case ENTITY_ENEMY:
            enemy_draw(e);
            break;
        case ENTITY_PROJECTILE:
            projectile_draw(e);
            break;
        default:
            if (e->sprite_id >= 0)
                draw_sprite(&sprites[e->sprite_id], e->x, e->y, 0, 0);
            break;
    }
}

void entity_erase_all(int cur_buf){
    /* erase - restore tiles under every entity*/
    for (int i = 0; i < MAX_ENTITIES; i++){
        if (!entities[i].active){
            continue;
        }
        erase_sprite(entities[i].prev_x[cur_buf] - 8, entities[i].prev_y[cur_buf], entities[i].width + 8, entities[i].height);

        if(cur_buf == 0 && entities[i].pending_erase_b1){
            entities[i].pending_erase_b1 = 0;
        }

        if(cur_buf == 1 && entities[i].pending_erase_b2){
            entities[i].pending_erase_b2 = 0;
        }

        if(entities[i].pending_erase && !entities[i].pending_erase_b1 && !entities[i].pending_erase_b2){
            entities[i].pending_erase = 0;
            entities[i].active = 0;
        }
    }
}

