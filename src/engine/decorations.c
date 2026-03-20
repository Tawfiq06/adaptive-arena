#include "decorations.h"
#include "map.h"
#include "vga.h"
#include "tile_sprites.h"
#include "sprite.h"
#include <stdlib.h>

/**************************************************/
/* DECOR LOOKUP MAP                               */
/* Same dimensions as tile map (15x20)            */
/* Each cell will have up to DECO_CELL_CAP decors */
/* built only ocne during init                    */
/**************************************************/

#define DECO_CELL_CAP 4

typedef struct{
    //this will hold which indicies in decor corrsepond to this cell
    unsigned char indices[DECO_CELL_CAP];
    unsigned char count; //amount of decor in this cell
} DecoCell;

DecoCell deco_map[MAP_HEIGHT][MAP_WIDTH];

void deco_map_build(void){
    //first cell every cell
    for(int r=0; r < MAP_HEIGHT; r++){
        for (int c = 0; c < MAP_WIDTH; c++){
            deco_map[r][c].count = 0;
        }
    }

    for(int i = 0; i < deco_count; i++){
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        //get all the cells it touches
        int c0 = decorations[i].x / TILE_W;
        int r0 = decorations[i].y /TILE_H;
        int c1 = (decorations[i].x + d->w - 1) / TILE_W;
        int r1 = (decorations[i].y + d->h - 1) / TILE_H;

        for(int r = r0; r <= r1 && r < MAP_HEIGHT; r++){
            for(int c = c0; c <= c1 && c < MAP_WIDTH; c++){
                if(r < 0 || c < 0){
                    continue;
                }
                DecoCell *cell = &deco_map[r][c];
                if(cell->count < DECO_CELL_CAP){
                    cell->indices[cell->count++] = (unsigned char)i;
                }
            }
        }
    }
}

void decoration_init(void){
    deco_count = 0;

    //map rools 0-19 to a specific type. -1 means no spawn
    const int SPAWN_TABLE[16] = {
        0, 1, 2, 3, 4, 5,       // Rocks (0-5)
        6, 7, 8, 9,             // Bushes (6-9)
        13, 14,                 // Flowers/Plants (10-11)
        12,                     // Stick (12)
        15, 16, 17,             // Trees (13-15)
    };

    while(deco_count < MAX_DECORATIONS){
        //get random tile
        int row = (rand() % MAP_WIDTH);
        int col = (rand() % MAP_HEIGHT);

        if(map_get_tile(row, col) != SPRITE_TILE_GRASS){
            continue; //if not grass no decor
        }

        int type = SPAWN_TABLE[rand() % 16];

        const DecoType* info = &DECO_LOOKUP[type];

        //random offset
        int px = (row * TILE_W) + (rand() % (TILE_W / 2));
        int py = (col * TILE_H) + (rand() % (TILE_H / 2));

        if(px + info->w > SCREEN_WIDTH){
            px = SCREEN_WIDTH - info->w;
        }
        if(py + info->h > SCREEN_HEIGHT){
            py = SCREEN_HEIGHT - info->h;
        }
        if(px < 0){
            px = 0;
        }
        if(py < 0){
            py = 0;
        }
        
        decorations[deco_count++] = (Decoration){ (short)px, (short)py, (unsigned char)type };
    }
    //now build the map
    deco_map_build();
}

void decoration_draw_all(void){
    for(int i = 0; i < deco_count; i++){
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        Sprite s = {
            d->w,
            d->h,
            d->data
        };
        draw_sprite(&s, decorations[i].x, decorations[i].y);
    }
}

//need to redraw decor if a sprite overlaps it
void decoration_redraw_region(int row0, int col0, int row1, int col1) {   
    int clip_x0 = col0 * TILE_W;
    int clip_x1 = col1 * TILE_W - 1;
    int clip_y0 = row0 * TILE_H;
    int clip_y1 = row1 * TILE_H - 1;
    
    for (int i = 0; i < deco_count; i++) {
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        int sx = decorations[i].x;
        int sy = decorations[i].y;
        /* Check overlaps with clipped — skip decorations with no pixels in the clip rect */
        if (sx + d->w <= clip_x0 || sx >= clip_x1 + 1) continue;
        if (sy + d->h <= clip_y0 || sy >= clip_y1 + 1) continue;
        /* Draw pixels inside the clip rect only */
        for (int row = 0; row < d->h; row++) {
            int py = sy + row;
            if (py < clip_y0 || py > clip_y1) continue;
            if (py < 0 || py >= SCREEN_HEIGHT) continue;
            for (int col = 0; col < d->w; col++) {
                int px = sx + col;
                if (px < clip_x0 || px > clip_x1) continue;
                if (px < 0 || px >= SCREEN_WIDTH) continue;
                short colour = d->data[row * d->w + col];
                if ((unsigned short)colour != (unsigned short)TRANSPARENT)
                    plot_pixel(px, py, colour);
            }
        }
    }
}