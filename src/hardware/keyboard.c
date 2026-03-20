#include "keyboard.h"
#include "address_map.h"

//pointer to base
volatile int *ps2 = (volatile int *)PS2_BASE;

/* Bitmask table — one bit per scancode (256 scancodes, 256/8 = 32 bytes) */
//there are 256 possible scancodes
unsigned char key_state[256 / 8];

void key_set(unsigned char code, int down) {
    if (down) key_state[code >> 3] |=  (1 << (code & 7)); //set to 1 if its pressed
    else      key_state[code >> 3] &= ~(1 << (code & 7)); //set to 0 if released
}

int key_pressed(unsigned char scancode) {
    return (key_state[scancode  >> 3] >> (scancode & 7)) & 1;
}

void keyboard_update(void) {
    static int break_next = 0; //next byte is a break (key-up) code 
    static int extended   = 0; //previous byte was 0xE0 prefix 

    int raw; //read once - bit 15 (RVALID)
    while ((raw = ps2[0]) & 0x8000) { //check bit 15 (RVALID), if 0 FIFO is empty
        unsigned char byte = (unsigned char)(raw & 0xFF); //take last 8 bits (the actual data)

        if (byte == 0xE0) { //warns its an exteneded key 
            extended = 1;
        } else if (byte == 0xF0) { // not a key, means its a key up event
            break_next = 1;
        } else {
            /* Extended keys (arrows etc.) use 0xE0 prefix — store them
               with bit 7 set so they don't collide with normal scancodes.
               e.g. arrow up: raw=0x75, stored as 0xF5 when extended.    */
            unsigned char code = extended ? (byte | 0x80) : byte;
            key_set(code, !break_next);
            break_next = 0;
            extended   = 0;
        }
    }
}