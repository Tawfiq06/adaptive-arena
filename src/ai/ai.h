#ifndef AI_H
#define AI_H
#include "../game/entity.h"

void ai_build_obs(float *obs, const Entity *agent, const Entity *target);
int  ai_nn_forward(const float *obs);
void ai_inject_input(Entity *agent, Entity *target);

#endif