#ifndef MAP_EVOLUTION_H
#define MAP_EVOLUTION_H

void map_evolution_init(int map_index);
void map_evolution_update(void);
void map_evolution_draw(int cur_buf);
void map_change_tile(int row, int col, SpriteID new_id, unsigned char new_flags);
void deco_change_type(int idx, int new_type);

#endif