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

    /* Define catergoies*/
    const int SMALL_TABLE[] = {
        DECO_ROCK_MED_BROWN, DECO_ROCK_SM_A_BROWN, DECO_ROCK_SM_B_BROWN, DECO_ROCK_SM_C_BROWN,
        DECO_ROCK_MED_GREY,  DECO_ROCK_SM_A_GREY,  DECO_ROCK_SM_B_GREY,  DECO_ROCK_SM_C_GREY,
        DECO_FLOWER_PURPLE,  DECO_PLANT_TALL,
    };

    const int BUSH_TABLE[] = {
        DECO_BUSH_GREEN_SM, DECO_BUSH_OLIVE_SM, DECO_BUSH_RED_SM,
        DECO_BUSH_GREEN_LG, DECO_BUSH_OLIVE_LG,
    };

    const int TREE_TABLE[] = {
        DECO_STICK_TREE,
        DECO_TREE_GREEN_A, DECO_TREE_GREEN_B,
        DECO_TREE_AUTUMN_A, DECO_TREE_AUTUMN_B,
    };

    const int SMALL_COUNT = 10;
    const int BUSH_COUNT  =  5;
    const int TREE_COUNT  =  5;

    /* decide how many of each */
    const int TREE_BUDGET = 3;
    const int BUSH_BUDGET = 5;
    const int SMALL_BUDGET = MAX_DECORATIONS - TREE_BUDGET - BUSH_BUDGET;

    int trees_placed = 0;
    int bushes_placed = 0;
    int smalls_placed = 0;

    /* Max placement attempts before giving up on a budget category */
    const int MAX_ATTEMPTS = 500;

    /* is this pixel position too close to an already-placed decoration? */
    #define MIN_SPACING 12   //pixels between decoration bounding boxes

    /* ---- place trees first (they need the most clear space) ---- */
    for(int attempts = 0; attempts < MAX_ATTEMPTS && trees_placed < TREE_BUDGET; attempts++){
        int row = 1 + (rand() % (MAP_HEIGHT - 2));  /* avoid border rows */
        int col = 1 + (rand() % (MAP_WIDTH  - 2));  /* avoid border cols */

        if(map_get_tile(row, col) != SPRITE_TILE_GRASS) continue;

        int type = TREE_TABLE[rand() % TREE_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];

        /* Place with a sub-tile random offset so trees don't always align to grid */
        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % (TILE_H / 2));

        /* Clamp to screen */
        if(px + info->w > SCREEN_WIDTH)  px = SCREEN_WIDTH  - info->w;
        if(py + info->h > SCREEN_HEIGHT) py = SCREEN_HEIGHT - info->h;
        if(px < 0) px = 0;
        if(py < 0) py = 0;

        /* Check tile under the BOTTOM of the sprite */
        int bot_col = px >> 4;
        int bot_row = (py + info->h - 1) >> 4;

        if(bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if(map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS) continue;

        /* Check spacing against already placed decorations */
        int too_close = 0;
        for(int i = 0; i < deco_count; i++){
            const DecoType *other = &DECO_LOOKUP[decorations[i].type];
            int dx = decorations[i].x - px;
            int dy = decorations[i].y - py;
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            if(dx < other->w + MIN_SPACING && dy < other->h + MIN_SPACING){
                too_close = 1;
                break;
            }
        }
        if(too_close) continue;

        decorations[deco_count++] = (Decoration){ (short)px, (short)py, (unsigned char)type };
        trees_placed++;
    }

    /* ---- place bushes ---- */
    for(int attempts = 0; attempts < MAX_ATTEMPTS && bushes_placed < BUSH_BUDGET; attempts++){
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));

        if(map_get_tile(row, col) != SPRITE_TILE_GRASS) continue;

        int type = BUSH_TABLE[rand() % BUSH_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];

        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % (TILE_H / 2));

        if(px + info->w > SCREEN_WIDTH)  px = SCREEN_WIDTH  - info->w;
        if(py + info->h > SCREEN_HEIGHT) py = SCREEN_HEIGHT - info->h;
        if(px < 0) px = 0;
        if(py < 0) py = 0;

        int bot_col = px >> 4;
        int bot_row = (py + info->h - 1) >> 4;
        
        if(bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if(map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS) continue;

        int too_close = 0;
        for(int i = 0; i < deco_count; i++){
            const DecoType *other = &DECO_LOOKUP[decorations[i].type];
            int dx = decorations[i].x - px;
            int dy = decorations[i].y - py;
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            if(dx < other->w + MIN_SPACING && dy < other->h + MIN_SPACING){
                too_close = 1;
                break;
            }
        }
        if(too_close) continue;

        decorations[deco_count++] = (Decoration){ (short)px, (short)py, (unsigned char)type };
        bushes_placed++;
    }

    /* ---- place small props (rocks, flowers) ---- */
    for(int attempts = 0; attempts < MAX_ATTEMPTS && smalls_placed < SMALL_BUDGET; attempts++){
        int row = 1 + (rand() % (MAP_HEIGHT - 2));
        int col = 1 + (rand() % (MAP_WIDTH  - 2));

        if(map_get_tile(row, col) != SPRITE_TILE_GRASS) continue;

        int type = SMALL_TABLE[rand() % SMALL_COUNT];
        const DecoType *info = &DECO_LOOKUP[type];

        int px = col * TILE_W + (rand() % TILE_W);
        int py = row * TILE_H + (rand() % TILE_H);  /* small props can sit anywhere in the tile */

        if(px + info->w > SCREEN_WIDTH)  px = SCREEN_WIDTH  - info->w;
        if(py + info->h > SCREEN_HEIGHT) py = SCREEN_HEIGHT - info->h;
        if(px < 0) px = 0;
        if(py < 0) py = 0;

        int bot_col = px >> 4;
        int bot_row = (py + info->h - 1) >> 4;

        if(bot_row < 0 || bot_row >= MAP_HEIGHT || bot_col < 0 || bot_col >= MAP_WIDTH) continue;
        if(map_get_tile(bot_row, bot_col) != SPRITE_TILE_GRASS) continue;

        int too_close = 0;
        for(int i = 0; i < deco_count; i++){
            const DecoType *other = &DECO_LOOKUP[decorations[i].type];
            int dx = decorations[i].x - px;
            int dy = decorations[i].y - py;
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            if(dx < other->w + MIN_SPACING && dy < other->h + MIN_SPACING){
                too_close = 1;
                break;
            }
        }
        if(too_close) continue;

        decorations[deco_count++] = (Decoration){ (short)px, (short)py, (unsigned char)type };
        smalls_placed++;
    }

    #undef MIN_SPACING

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
        draw_sprite(&s, decorations[i].x, decorations[i].y, 0, 0);
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