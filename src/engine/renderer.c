#include "renderer.h"
#include "sprite.h"
#include "map.h"
#include "vga.h"
#include "tile_sprites.h"
#include "decorations.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

void draw_sprite(const Sprite *s, int x, int y, int flip_h, int flip_v){
    for(int row = 0; row < s->height; row++)
    {   
        int py = y;
        if(flip_v){
            py += s->height - 1 -row;
        }
        else{
            py += row;
        }

        if(py < 0 || py >= SCREEN_HEIGHT) continue;

        for(int col = 0; col < s->width; col++)
        {   
            int px = x;
            if(flip_h){
                px += s->width - 1 - col;
            }
            else{
                px += col;
            }

            if(px < 0 || px >= SCREEN_WIDTH) continue;

            int idx = row * s->width + col;
            short colour = s->data[idx];

            if((unsigned short) colour != (unsigned short) TRANSPARENT) // transparency
            {
                plot_pixel(px, py, colour);
            }
        }
    }
}

void draw_background(){
    for (int row = 0; row < MAP_HEIGHT; row++){
        for(int col = 0; col < MAP_WIDTH; col++){
            SpriteID id = map_get_tile(row, col);

            //calculate postion
            int x_pos = col << 4; //current map tile * 16 give the coord
            int y_pos = row << 4;

            draw_sprite(&sprites[id], x_pos, y_pos, 0, 0);
        }
    }
}

void erase_sprite(int x, int y, int w, int h){
    //convert pixel rect to tile indices
    int col0 = x >> 4;
    int col1 = (x + w + TILE_W - 1) >> 4;
    int row0 = y >> 4;
    int row1 = (y + h + TILE_H - 1) >> 4;

    if (col0 < 0)          col0 = 0;
    if (row0 < 0)          row0 = 0;
    if (col1 > MAP_WIDTH)  col1 = MAP_WIDTH;
    if (row1 > MAP_HEIGHT) row1 = MAP_HEIGHT;

    /*Handle background tiles*/
    for(int row = row0; row < row1; row++){
        for (int col = col0; col < col1; col++){
            SpriteID id = map_get_tile(row, col);
            draw_sprite(&sprites[id], col << 4, row << 4, 0, 0);
        }
    }

    /*Now handle redrawing decorations*/
    decoration_redraw_region(row0, col0, row1, col1);

    //requeue
}