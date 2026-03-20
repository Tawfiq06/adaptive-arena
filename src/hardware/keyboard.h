#ifndef KEYBOARD_H
#define KEYBOARD_H

//Arrow keys are extened (0xE0 prefix) - stores with bit 7 set
#define KEY_UP      0xF5
#define KEY_DOWN    0xF2
#define KEY_LEFT    0xEB
#define KEY_RIGHT   0xF4

#define KEY_SPACE   0x29
#define KEY_ESC     0x76
#define KEY_LSHIFT  0x12
#define KET_RSHIFT  0x59
#define KEY_LCTRL   0x14
#define KEY_CAPS    0x58
#define KEY_ENTER   0x5A
#define KET_BKSP    0x66 

#define KEY_A       0x1C
#define KEY_B       0x32
#define KEY_C       0x21
#define KEY_D       0x23
#define KEY_E       0x24
#define KEY_F       0x2B    
#define KEY_G       0x34    
#define KEY_H       0x33    
#define KEY_I       0x43    
#define KEY_J       0x3B
#define KEY_K       0x42    
#define KEY_L       0x4B    
#define KEY_M       0x3A    
#define KEY_N       0x31    
#define KEY_O       0x44
#define KEY_P       0x4D    
#define KEY_Q       0x15    
#define KEY_R       0x2D    
#define KEY_S       0x1B    
#define KEY_T       0x2C
#define KEY_U       0x3C    
#define KEY_V       0x2A    
#define KEY_W       0x1D    
#define KEY_X       0x22    
#define KEY_Y       0x35
#define KEY_Z       0x1A

#define KEY_1       0x16    
#define KEY_2       0x1E    
#define KEY_3       0x26    
#define KEY_4       0x25    
#define KEY_5       0x2E
#define KEY_6       0x36    
#define KEY_7       0x3D    
#define KEY_8       0x3E    
#define KEY_9       0x46    
#define KEY_0       0x45
/* Returns 1 if the key with the given scancode is currently held down */
int key_pressed(unsigned char scancode);

/* Call once per frame to drain the PS/2 FIFO and update key state */
void keyboard_update(void);

#endif