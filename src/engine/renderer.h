#ifndef RENDERER_H
#define RENDERER_H

#include "sprite.h"

void draw_background();
void draw_sprite(const Sprite *s, int x, int y, int flip_h, int flip_v);
void erase_sprite(int x, int y, int w, int h);

#endif