#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "soldier_frames.h"

#define GAME_FPS 60

typedef struct{
    unsigned char anim; //current animation
    unsigned char frame; //current animation frame
    unsigned char tick; //count down to next frame
    unsigned char flip; //to mirror (Ex facing left)
} Animator;

static inline void anim_init(Animator *a, const AnimDef *defs, int start_anim);

static inline int anim_tick(Animator *a, const AnimDef *defs);

static inline const short *anim_frame(const Animator *a);

static inline void anim_play(Animator *a, const AnimDef *defs, int id);

static inline void anim_restart(Animator *a, const AnimDef *def);

#endif