#ifndef GAME_H
#define GAME_H

extern int game_winner; //0 = no winner, 1 p1 wins, 2 p2 wins

void game_init();
void update_game(int cur_buf);
void draw_game(int cur_buf);

void ai_get_nearest_potion(int agent_x, int agent_y,
                           float *out_rx, float *out_ry, int *out_active);
#endif