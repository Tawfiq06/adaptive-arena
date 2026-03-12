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