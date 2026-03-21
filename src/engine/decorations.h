#ifndef DECORATIONS_H
#define DECORATIONS_H

#include "decoration_sprites.h"

#define MAX_DECORATIONS 15
#define DECO_CELL_CAP 4

typedef struct{
    short x, y;
    unsigned char type;
} Decoration;

extern int deco_count;

typedef struct{
    //this will hold which indicies in decor corrsepond to this cell
    unsigned char indices[DECO_CELL_CAP];
    unsigned char count; //amount of decor in this cell
} DecoCell;

extern Decoration decorations[MAX_DECORATIONS];
const DecoCell *deco_map_get_cell(int row, int col);

void decoration_init(void);
void decoration_redraw_region(int row0, int col0, int row1, int col1);
#endif