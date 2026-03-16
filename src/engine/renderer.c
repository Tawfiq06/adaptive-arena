#include "renderer.h"
#include "sprite.h"
#include "map.h"
#include "vga.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

void draw_sprite(const Sprite *s, int x, int y){
    for(int row = 0; row < s->height; row++)
    {   
        int py = y + row;
        if(py < 0 || py >= SCREEN_HEIGHT) continue;

        for(int col = 0; col < s->width; col++)
        {   
            int px = x + col;
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

            draw_sprite(&sprites[id], x_pos, y_pos);
        }
    }
}

void erase_sprite(int x, int y, int w, int h){
    //convert pixel rect to tile indices
    int col0 = x >> 4;
    int col1 = (x + w + 15) >> 4;
    int row0 = y >> 4;
    int row1 = (y + h + 15) >> 4;

    for(int row = row0; row < row1; row++){
        for (int col = col0; col < col1; col++){
            SpriteID id = map_get_tile(row, col);
            draw_sprite(&sprites[id], col << 4, row << 4);
        }
    }
}