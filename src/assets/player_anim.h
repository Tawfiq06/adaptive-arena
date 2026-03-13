#ifndef PLAYER_ANIM_H
#define PLAYER_ANIM_H

#include "soldier_frames.h"

#define GAME_FPS 60

typedef struct{
    unsigned char anim; //current animation
    unsigned char frame; //current animation frame
    unsigned char tick; //count down to next frame
    unsigned char flip; //to mirror (Ex facing left)
} PlayerAnim;

static inline void anim_init(PlayerAnim *a);

static inline int anim_tick(PlayerAnim *a);

static inline const short *anim_frame(const PlayerAnim *a);

static inline void anim_play(PlayerAnim *a, AnimID id);

static inline void anim_restart(PlayerAnim *a);

#endif