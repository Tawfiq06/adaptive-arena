#ifndef DECORATIONS_H
#define DECORATIONS_H

#include "decoration_sprites.h"

#define MAX_DECORATIONS 96

typedef struct{
    short x, y;
    unsigned char type;
} Decoration;

int deco_count;

Decoration decorations[MAX_DECORATIONS];
void decoration_init(void);
void decoration_redraw_region(int x, int y, int w, int h);
#endif