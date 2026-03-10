#include "renderer.h"
#include "sprite.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

void draw_sprite(const Sprite *s, int x, int y){
    for(int row = 0; row < s->height; row++)
    {
        if(y + row < 0 || y + row >= SCREEN_HEIGHT) continue;

        for(int col = 0; col < s->width; col++)
        {   
            if(x + col < 0 || x + col >= SCREEN_WIDTH) continue;

            int idx = row * s->width + col;
            short colour = s->data[idx];

            if(colour != TRANSPARENT) // transparency
            {
                plot_pixel(x + col, y + row, colour);
            }
        }
    }
}