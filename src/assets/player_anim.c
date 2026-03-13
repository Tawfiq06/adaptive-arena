#include "player_anim.h"
#include "vga.h"

static void anim_init(PlayerAnim *a){
    a->anim = ANIM_IDLE;
    a->frame = SOLDIER_ANIMS[ANIM_IDLE].start;
    a->tick = GAME_FPS / SOLDIER_ANIMS[ANIM_IDLE].fps;
    a->flip = 0;
}

/*Returns 1 when a non loop animation finishes its last frame*/
static inline int anim_tick(PlayerAnim *a) {
    if (a->tick > 0) { a->tick--; return 0; }

    const AnimDef *def = &SOLDIER_ANIMS[a->anim];
    a->tick = GAME_FPS / def->fps;

    if (a->frame < def->end) {
        a->frame++;
        return 0;
    }

    //reached the last frame
    if (def->loop) {
        a->frame = def->start;
        return 0;
    }
    return 1; 
}

static inline const short *anim_frame(const PlayerAnim *a) {
    return soldier_frames[a->frame];
}


static inline void anim_play(PlayerAnim *a, AnimID id) {
    if (a->anim == id) return;
    a->anim  = (unsigned char)id;
    a->frame = SOLDIER_ANIMS[id].start;
    a->tick  = GAME_FPS / SOLDIER_ANIMS[id].fps;
}

static inline void anim_restart(PlayerAnim *a) {
    a->frame = SOLDIER_ANIMS[a->anim].start;
    a->tick  = GAME_FPS / SOLDIER_ANIMS[a->anim].fps;
}

static void draw_soldier(const PlayerAnim *a, int x, int y){
    const short *frame = soldier_frames[a->frame];
    for (int row = 0; row < SOLDIER_H; row++){
        int py = y + row;
        if(py < 0 || py >= SCREEN_HEIGHT){
            continue;
        }
        for(int col = 0; col < SOLDIER_W; col++){
            int px = x + (a->flip ? (SOLDIER_W - 1 - col) : col);
            if (px < 0 || px >= SCREEN_WIDTH){
                continue;
            }
            short colour = frame[row * SOLDIER_W + col];
            if ((unsigned short)colour != (unsigned short)TRANSPARENT){
                plot_pixel(px, py, colour);
            }
        }
    }
}