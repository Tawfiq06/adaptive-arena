#ifndef DECORATIONS_H
#define DECORATIONS_H

#include "decoration_sprites.h"

#define MAX_DECORATIONS 2

typedef struct{
    short x, y;
    unsigned char type;
} Decoration;

int deco_count;

Decoration decorations[MAX_DECORATIONS];
void decoration_init(void);
void decoration_redraw_region(int row0, int col0, int row1, int col1);
#endif