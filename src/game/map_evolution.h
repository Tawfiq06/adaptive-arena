#ifndef MAP_EVOLUTION_H
#define MAP_EVOLUTION_H

void map_evolution_init(int map_index);
void map_evolution_update(void);
void map_evolution_draw(int cur_buf);
void map_change_tile(int row, int col, SpriteID new_id, unsigned char new_flags);
void deco_change_type(int idx, int new_type);

#define TILE_REDRAW_CAP 75

typedef struct{
    short row, col;
    int draw_b0;  //needs redraw into buffer 2
    int draw_b1; //needs redraw into buffer 1
} TileRedraw;

TileRedraw tile_redraws[TILE_REDRAW_CAP];
int tile_redraw_count;

#define DECO_REDRAW_CAP 15


typedef struct{
    int deco_idx;
    int draw_b0;
    int draw_b1;
    //bounding box to erase old sprite
    short erase_col0, erase_row0;
    short erase_col1, erase_row1;
} DecoRedraw;

DecoRedraw deco_redraws[DECO_REDRAW_CAP];
int deco_redraw_count;

#endif