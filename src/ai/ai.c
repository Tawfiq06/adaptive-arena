#include "ai.h"
#include "weights.h"
#include "../game/game.h"   
#include "../game/player.h" 
#include <math.h>
#include "vga.h"

#define CLAMP(x, lo, hi) ((x)<(lo)?(lo):(x)>(hi)?(hi):(x))

/* Action indices — must match env.py action space */
#define ACT_UP    0
#define ACT_DOWN  1
#define ACT_LEFT  2
#define ACT_RIGHT 3
#define ACT_ATK1  4
#define ACT_ATK2  5
#define ACT_SHOOT 6
#define ACT_DASH  7
#define ACT_BLOCK 8
/* 9 = idle */

static float relu(float x) { return x > 0.0f ? x : 0.0f; }

void ai_build_obs(float *obs, const Entity *agent, const Entity *target) {
    float pot_rx, pot_ry;
    int   pot_active;
    ai_get_nearest_potion(agent->x, agent->y, &pot_rx, &pot_ry, &pot_active);

    /* Arrow danger: scan entities for active projectiles not owned by agent */
    float arrow_danger = 0.0f;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) continue;
        if (entities[i].type != ENTITY_PROJECTILE) continue;
        if (entities[i].owner == agent) continue;
        int dx = entities[i].x - agent->x;
        float dist = (float)(dx < 0 ? -dx : dx) / SCREEN_WIDTH;
        if (dist < arrow_danger || arrow_danger == 0.0f)
            arrow_danger = dist;
    }
    if (arrow_danger > 1.0f) arrow_danger = 0.0f;

    obs[0]  = (float)agent->x  / SCREEN_WIDTH  * 2.0f - 1.0f;
    obs[1]  = (float)agent->y  / SCREEN_HEIGHT * 2.0f - 1.0f;
    obs[2]  = (float)target->x / SCREEN_WIDTH  * 2.0f - 1.0f;
    obs[3]  = (float)target->y / SCREEN_HEIGHT * 2.0f - 1.0f;
    obs[4]  = CLAMP((float)agent->health  / 100.0f, -1.0f, 1.0f);
    obs[5]  = CLAMP((float)target->health / 100.0f, -1.0f, 1.0f);
    obs[6]  = (float)agent->atk1_cooldown  / 20.0f;
    obs[7]  = (float)agent->atk2_cooldown  / 30.0f;
    obs[8]  = (float)agent->shoot_cooldown / 45.0f;
    obs[9]  = (agent->facing == 'e') ? 1.0f : -1.0f;
    obs[10] = (float)agent->dash_cooldown  / 60.0f;
    obs[11] = agent->is_dashing ? 1.0f : 0.0f;
    obs[12] = pot_rx;
    obs[13] = pot_ry;
    obs[14] = (float)pot_active;
    obs[15] = arrow_danger;
}

int ai_nn_forward(const float *obs) {
    float h1[NN_HIDDEN_SIZE], h2[NN_HIDDEN_SIZE], logits[NN_OUTPUT_SIZE];

    for (int i = 0; i < NN_HIDDEN_SIZE; i++) {
        float sum = bias_0[i];   /* bias is separate from weights */
        for (int j = 0; j < NN_INPUT_SIZE; j++)
            sum += weight_0[i * NN_INPUT_SIZE + j] * obs[j];
        h1[i] = relu(sum);
    }
    for (int i = 0; i < NN_HIDDEN_SIZE; i++) {
        float sum = bias_2[i];
        for (int j = 0; j < NN_HIDDEN_SIZE; j++)
            sum += weight_2[i * NN_HIDDEN_SIZE + j] * h1[j];
        h2[i] = relu(sum);
    }

    int best = 0; float best_val = -1e9f;
    for (int i = 0; i < NN_OUTPUT_SIZE; i++) {
        float sum = action_bias[i];
        for (int j = 0; j < NN_HIDDEN_SIZE; j++)
            sum += action_weight[i * NN_HIDDEN_SIZE + j] * h2[j];
        logits[i] = sum;
        if (sum > best_val) { best_val = sum; best = i; }
    }
    return best;
}

void ai_inject_input(Entity *agent, Entity *target) {
    float obs[NN_INPUT_SIZE];
    ai_build_obs(obs, agent, target);
    int action = ai_nn_forward(obs);

    /* Reset movement — mirrors what player_update does before key_pressed */
    if (!agent->is_dashing) { agent->dx = 0; agent->dy = 0; }

    switch (action) {
        case ACT_UP:    agent->dy = -PLAYER_SPEED; break;
        case ACT_DOWN:  agent->dy =  PLAYER_SPEED; break;
        case ACT_LEFT:
            agent->dx = -PLAYER_SPEED;
            agent->facing = 'w';
            agent->anim.flip = 1;
            break;
        case ACT_RIGHT:
            agent->dx =  PLAYER_SPEED;
            agent->facing = 'e';
            agent->anim.flip = 0;
            break;
        case ACT_ATK1:
            if (agent->atk1_cooldown == 0) {
                anim_play(&agent->anim, agent->anim_def, SOLDIER_ATK1);
                agent->atk1_cooldown = ATK1_COOLDOWN;
                agent->attack_s1 = 1;
            }
            break;
        case ACT_ATK2:
            if (agent->atk2_cooldown == 0) {
                anim_play(&agent->anim, agent->anim_def, SOLDIER_ATK2);
                agent->atk2_cooldown = ATK2_COOLDOWN;
                agent->attack_s2 = 1;
            }
            break;
        case ACT_SHOOT:
            if (agent->shoot_cooldown == 0) {
                anim_play(&agent->anim, agent->anim_def, SOLDIER_ATK3);
                agent->shoot_cooldown = SHOOT_COOLDOWN;
                agent->arrow_fired = 0;
                agent->attack_p = 0;
            }
            break;
        case ACT_DASH:
            if (agent->dash_cooldown == 0) {
                agent->dash_cooldown = DASH_COOLDOWN;
                agent->is_dashing = 1;
                agent->dash_timer = DASH_TIMER;
            }
            break;
        case ACT_BLOCK:
            if (agent->block_cooldown == 0) {
                agent->block_cooldown = BLOCK_COOLDOWN;
                agent->blocking = 1;
                agent->block_timer = BLOCK_TIMER;
            }
            break;
        default: break; /* idle */
    }
}