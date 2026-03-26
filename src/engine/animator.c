#include "animator.h"
#include "vga.h"

void anim_init(Animator *a, const AnimDef *def, int start_anim){
    a->anim = (unsigned char) start_anim;
    a->frame = def[start_anim].start;
    a->tick = GAME_FPS / def[start_anim].fps;
    a->flip = 0;
    
}

/*Returns 1 when a non loop animation finishes its last frame*/
int anim_tick(Animator *a, const AnimDef *defs) {
    if (a->tick > 0) { a->tick--; return 0; }

    const AnimDef *def = &defs[a->anim];
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

const short *anim_frame(const Animator *a, const short * const *frames) {
    return frames[a->frame];
}


void anim_play(Animator *a, const AnimDef *def, int id) {
    if (a->anim == (unsigned char) id) return;
    a->anim  = (unsigned char)id;
    a->frame = def[id].start;
    a->tick  = GAME_FPS / def[id].fps;
}

void anim_restart(Animator *a, const AnimDef *def) {
    a->frame = def[a->anim].start;
    a->tick  = GAME_FPS / def[a->anim].fps;
}

void draw_soldier(const Animator *a, int x, int y){
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