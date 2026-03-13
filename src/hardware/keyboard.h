#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Scancodes for keys we care about (Set 2) */
#define KEY_W     0x1D
#define KEY_A     0x1C
#define KEY_S     0x1B
#define KEY_D     0x23

//Arrow keys are extened (0xE0 prefix) - stores with bit 7 set
#define KEY_UP    0xF5
#define KEY_DOWN  0xF2
#define KEY_LEFT  0xEB
#define KEY_RIGHT 0xF4

#define KEY_SPACE 0x29
#define KEY_ESC   0x76

#define KEY_Z     0x1A  /* attack 1 */
#define KEY_X     0x22  /* attack 2 */

/* Returns 1 if the key with the given scancode is currently held down */
int key_pressed(unsigned char scancode);

/* Call once per frame to drain the PS/2 FIFO and update key state */
void keyboard_update(void);

#endif