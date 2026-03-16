/* soldier_frames.h — generated from Soldier.aseprite
 * Tight crop: 41x29px per frame (was 100x100, 88% empty removed)
 * Format: RGB565, 0xF81F = transparent (magenta key)
 * All frames are const -> placed in SDRAM .rodata, zero on-chip SRAM cost
 * Per-entity runtime state: ~16 bytes (just indices)
 */
#ifndef SOLDIER_FRAMES_H
#define SOLDIER_FRAMES_H

#define SOLDIER_W      41
#define SOLDIER_H      29
#define SOLDIER_FRAMES 43

#include "sprite.h"

typedef enum {
    SOLIDER_IDLE,
    SOLDIER_WALK,
    SOLDIER_ATK1,
    SOLDIER_ATK2,
    SOLDIER_RUN,
    SOLDIER_HURT,
    SOLDIER_DEATH,
    SOLDIER_COUNT
} SoldierID;

typedef struct {
    unsigned char start;  /* first frame index */
    unsigned char end;    /* last frame index (inclusive) */
    unsigned char loop;   /* 1=loop, 0=play once */
    unsigned char fps;
} AnimDef;

extern const AnimDef SOLDIER_ANIMS[SOLDIER_COUNT];
extern const short soldier_f00[41*29];
extern const short soldier_f01[41*29];
extern const short soldier_f02[41*29];
extern const short soldier_f03[41*29];
extern const short soldier_f04[41*29];
extern const short soldier_f05[41*29];
extern const short soldier_f06[41*29];
extern const short soldier_f07[41*29];
extern const short soldier_f08[41*29];
extern const short soldier_f09[41*29];
extern const short soldier_f10[41*29];
extern const short soldier_f11[41*29];
extern const short soldier_f12[41*29];
extern const short soldier_f13[41*29];
extern const short soldier_f14[41*29];
extern const short soldier_f15[41*29];
extern const short soldier_f16[41*29];
extern const short soldier_f17[41*29];
extern const short soldier_f18[41*29];
extern const short soldier_f19[41*29];
extern const short soldier_f20[41*29];
extern const short soldier_f21[41*29];
extern const short soldier_f22[41*29];
extern const short soldier_f23[41*29];
extern const short soldier_f24[41*29];
extern const short soldier_f25[41*29];
extern const short soldier_f26[41*29];
extern const short soldier_f27[41*29];
extern const short soldier_f28[41*29];
extern const short soldier_f29[41*29];
extern const short soldier_f30[41*29];
extern const short soldier_f31[41*29];
extern const short soldier_f32[41*29];
extern const short soldier_f33[41*29];
extern const short soldier_f34[41*29];
extern const short soldier_f35[41*29];
extern const short soldier_f36[41*29];
extern const short soldier_f37[41*29];
extern const short soldier_f38[41*29];
extern const short soldier_f39[41*29];
extern const short soldier_f40[41*29];
extern const short soldier_f41[41*29];
extern const short soldier_f42[41*29];

extern const short * const soldier_frames[SOLDIER_FRAMES];


#endif