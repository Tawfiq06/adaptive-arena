#ifndef DECORATIONS_H
#define DECORATIONS_H

#include "decoration_sprites.h"

#define MAX_DECORATIONS 20
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

extern int canopy_indices[MAX_DECORATIONS];
extern int canopy_count;

extern Decoration decorations[MAX_DECORATIONS];
const DecoCell *deco_map_get_cell(int row, int col);

void decoration_init(int map_index);
void decoration_redraw_region(int row0, int col0, int row1, int col1);
void decoration_draw_canopies(void);
void decoration_draw_canopies_near(int px1, int py1, int px2, int py2);
#endif