#include "decorations.h"
#include "map.h"
#include "vga.h"
#include "tile_sprites.h"
#include "sprite.h"
#include <stdlib.h>

static void decoration_init(void){
    deco_count = 0;

    //map rools 0-19 to a specific type. -1 means no spawn
    static const int SPAWN_TABLE[16] = {
        0, 1, 2, 3, 4, 5,       // Rocks (0-5)
        6, 7, 8, 9,             // Bushes (6-9)
        13, 14,                 // Flowers/Plants (10-11)
        12,                     // Stick (12)
        15, 16, 17,             // Trees (13-15)
    };

    while(deco_count < MAX_DECORATIONS){
        //get random tile
        int x = (rand() % MAP_WIDTH);
        int y = (rand() % MAP_HEIGHT);

        if(map_get_tile(x, y) != SPRITE_TILE_GRASS){
            continue; //if not grass no decor
        }

        int type = SPAWN_TABLE[rand() % 20];

        const DecoType* info = &DECO_LOOKUP[type];

        //random offset
        int px = (x * TILE_W) + (rand() % (TILE_W / 2));
        int py = (y * TILE_H) + (rand() % (TILE_H / 2));

        px = (px + info->w > SCREEN_WIDTH)  ? SCREEN_WIDTH  - info->w : (px < 0 ? 0 : px);
        py = (py + info->h > SCREEN_HEIGHT) ? SCREEN_HEIGHT - info->h : (py < 0 ? 0 : py);
        
        decorations[deco_count++] = (Decoration){ (short)px, (short)py, (unsigned char)type };
    }
}

static void decoration_draw_all(void){
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
static void decoration_redraw_region(int x, int y, int w, int h) {
    for (int i = 0; i < deco_count; i++) {
        const DecoType *d = &DECO_LOOKUP[decorations[i].type];
        int dx = decorations[i].x;
        int dy = decorations[i].y;
        /* AABB overlap test */
        if (dx + d->w <= x || dx >= x + w) continue;
        if (dy + d->h <= y || dy >= y + h) continue;
        Sprite s = { d->w, d->h, d->data };
        draw_sprite(&s, dx, dy);
    }
}