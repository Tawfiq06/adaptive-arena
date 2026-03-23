#ifndef PLAYER_CONFIG_H
#define PLAYER_CONFIG_H

#include "keyboard.h"

/* ------------------------------------------------------------------
 * PlayerConfig — one of these per player.
 * To re-bind keys, just change the values here
 * ------------------------------------------------------------------ */
typedef struct {
    unsigned char key_up;
    unsigned char key_down;
    unsigned char key_left;
    unsigned char key_right;
    unsigned char key_atk1;
    unsigned char key_atk2;
    unsigned char key_atkp;
    unsigned char key_dash;
    unsigned char key_block;
    int health_x;
    int health_y;
} PlayerConfig;

/* Player 1: WASD + Z/X attacks */
#define PLAYER1_CONFIG { \
    .key_up    = KEY_W,     \
    .key_down  = KEY_S,     \
    .key_left  = KEY_A,     \
    .key_right = KEY_D,     \
    .key_atk1  = KEY_Q,     \
    .key_atk2  = KEY_E,      \
    .key_atkp = KEY_R,   \
    .key_dash = KEY_Z,   \
    .key_block = KEY_X,   \
    .health_x = 20,    \
    .health_y = 20    \
}

/* Player 2: Arrow keys + Q/E attacks */
#define PLAYER2_CONFIG { \
    .key_up    = KEY_UP,    \
    .key_down  = KEY_DOWN,  \
    .key_left  = KEY_LEFT,  \
    .key_right = KEY_RIGHT, \
    .key_atk1  = KEY_M,     \
    .key_atk2  = KEY_K,      \
    .key_atkp = KEY_L,     \
    .key_dash = KEY_N,       \
    .key_block = KEY_J,      \
    .health_x = 270,     \  //manual calculations for (SCREEN_WIDTH - 20 - full_health)
    .health_y = 20     \
}

#endif
