/*
 * adaptive-arena — flattened single file for CPulator
 * Target: DE1-SoC (RISC-V RV32), VGA 320x240, 60 FPS timer interrupt
 */

/* ===========================================================================
 * EXCEPTION HANDLER — must be first so linker places it at 0x00000000
 * naked = no compiler prologue/epilogue; we save/restore manually.
 * =========================================================================*/
void __attribute__((section(".exceptions"), naked)) exception_handler(void) {
    asm volatile(
        "addi sp, sp, -64\n"
        "sw   ra,  0(sp)\n" "sw   t0,  4(sp)\n" "sw   t1,  8(sp)\n"
        "sw   t2, 12(sp)\n" "sw   a0, 16(sp)\n" "sw   a1, 20(sp)\n"
        "sw   a2, 24(sp)\n" "sw   a3, 28(sp)\n" "sw   a4, 32(sp)\n"
        "sw   a5, 36(sp)\n" "sw   a6, 40(sp)\n" "sw   a7, 44(sp)\n"
        "sw   t3, 48(sp)\n" "sw   t4, 52(sp)\n" "sw   t5, 56(sp)\n"
        "sw   t6, 60(sp)\n"

        "csrr t0, mcause\n"
        "bgez t0, 1f\n"           /* bit 31 clear = exception, not interrupt */

        "slli t0, t0, 1\n"        /* strip bit 31, leaving IRQ number */
        "srli t0, t0, 1\n"
        "li   t1, 16\n"           /* IRQ 16 = interval timer */
        "bne  t0, t1, 1f\n"

        "lui  t2, 0xFF202\n"      /* TIMER_BASE — clear TO bit */
        "li   t3, 1\n"
        "sw   t3, 0(t2)\n"
        "la   t2, frame_flag\n"   /* frame_flag = 1 */
        "sw   t3, 0(t2)\n"

        "1:\n"
        "lw   ra,  0(sp)\n" "lw   t0,  4(sp)\n" "lw   t1,  8(sp)\n"
        "lw   t2, 12(sp)\n" "lw   a0, 16(sp)\n" "lw   a1, 20(sp)\n"
        "lw   a2, 24(sp)\n" "lw   a3, 28(sp)\n" "lw   a4, 32(sp)\n"
        "lw   a5, 36(sp)\n" "lw   a6, 40(sp)\n" "lw   a7, 44(sp)\n"
        "lw   t3, 48(sp)\n" "lw   t4, 52(sp)\n" "lw   t5, 56(sp)\n"
        "lw   t6, 60(sp)\n"
        "addi sp, sp, 64\n"
        "mret\n"
    );
}

#include <stdlib.h>
#include <stdbool.h>

/* ===========================================================================
 * ADDRESS MAP
 * =========================================================================*/
#define PIXEL_BUF_CTRL_BASE  0xFF203020
#define TIMER_BASE           0xFF202000
#define PS2_BASE             0xFF200100

/* ===========================================================================
 * VGA
 * =========================================================================*/
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define BUFFER_WIDTH  512

volatile int pixel_buffer_start;
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];

static void _swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

void wait_for_vsync() {
    volatile int *ctrl = (volatile int *)PIXEL_BUF_CTRL_BASE;
    *ctrl = 1;                          /* request buffer swap */
    while (*(ctrl + 3) & 0x1);         /* wait for S bit to clear */
    pixel_buffer_start = *(ctrl + 1);  /* update back-buffer pointer */
}

void vga_init() {
    volatile int *ctrl = (volatile int *)PIXEL_BUF_CTRL_BASE;
    *ctrl = 0;
    *(ctrl + 1) = (int)Buffer1;
    wait_for_vsync();
    pixel_buffer_start = *ctrl;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = 0;
    }
    *(ctrl + 1) = (int)Buffer2;
    pixel_buffer_start = *(ctrl + 1);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = 0;
    }
}

void plot_pixel(int x, int y, short colour) {
    volatile short *addr = (volatile short *)(pixel_buffer_start + (y << 10) + (x << 1));
    *addr = colour;
}

void draw_rect(int x, int y, int w, int h, short colour) {
    int x1 = x + w, y1 = y + h;
    if (x  < 0)             x  = 0;
    if (y  < 0)             y  = 0;
    if (x1 > SCREEN_WIDTH)  x1 = SCREEN_WIDTH;
    if (y1 > SCREEN_HEIGHT) y1 = SCREEN_HEIGHT;
    for (int row = y; row < y1; row++) {
        volatile short *rp = (volatile short *)(pixel_buffer_start + (row << 10));
        for (int col = x; col < x1; col++) rp[col] = colour;
    }
}

void draw_line(int x0, int y0, int x1, int y1, short colour) {
    bool steep = abs(y1-y0) > abs(x1-x0);
    if (steep)   { _swap(&x0,&y0); _swap(&x1,&y1); }
    if (x0 > x1) { _swap(&x0,&x1); _swap(&y0,&y1); }
    int dx = x1-x0, dy = abs(y1-y0), err = -(dx/2);
    int y = y0, ystep = (y0 < y1) ? 1 : -1;
    for (int x = x0; x <= x1; x++) {
        if (steep) plot_pixel(y, x, colour);
        else       plot_pixel(x, y, colour);
        err += dy;
        if (err > 0) { y += ystep; err -= dx; }
    }
}

/* ===========================================================================
 * TIMER
 * =========================================================================*/
#define FPS        60
#define TIMER_FREQ 100000000

volatile int frame_flag = 0;
static volatile unsigned int *timer_ptr = (volatile unsigned int *)TIMER_BASE;

void timer_init() {
    unsigned int period = TIMER_FREQ / FPS;
    timer_ptr[0] = 1;               /* clear any pending TO */
    timer_ptr[1] = 0x8;             /* stop timer */
    timer_ptr[2] = period & 0xFFFF;
    timer_ptr[3] = period >> 16;
    asm volatile("csrs mie, %0" : : "r"(0x10000)); /* enable IRQ16 in mie */
    timer_ptr[1] = 0x7;             /* START | CONT | ITO */
}

/* ===========================================================================
 * KEYBOARD  (PS/2 Set 2 scancodes, memory-mapped polling)
 *
 * Register layout (from Intel PS/2 Core datasheet):
 *   offset 0 (data reg):    [31:16] RAVAIL  [15] RVALID  [7:0] DATA
 *   offset 4 (control reg): [10] CE  [8] RI  [0] RE
 *
 * CRITICAL: each read of the data register pops one byte from the FIFO.
 * Always read into a local variable first — never read twice in one expression.
 *
 * Extended keys (arrows, etc.) send a 0xE0 prefix byte before their scancode.
 * We store extended scancodes with bit 7 set so make and break codes both
 * resolve to the same value and don't collide with normal scancodes.
 *   e.g.  Arrow Up   raw=0x75  →  stored as 0xF5  (0x75 | 0x80)
 *         Arrow Down  raw=0x72  →  stored as 0xF2
 *         Arrow Left  raw=0x6B  →  stored as 0xEB
 *         Arrow Right raw=0x74  →  stored as 0xF4
 * =========================================================================*/
#define KEY_W     0x1D
#define KEY_A     0x1C
#define KEY_S     0x1B
#define KEY_D     0x23
#define KEY_SPACE 0x29
#define KEY_ESC   0x76
#define KEY_UP    0xF5
#define KEY_DOWN  0xF2
#define KEY_LEFT  0xEB
#define KEY_RIGHT 0xF4

static volatile int *ps2_ptr = (volatile int *)PS2_BASE;
static unsigned char key_state[256 / 8]; /* 1 bit per scancode */

static void key_set(unsigned char code, int down) {
    if (down) key_state[code >> 3] |=  (1 << (code & 7));
    else      key_state[code >> 3] &= ~(1 << (code & 7));
}

int key_pressed(unsigned char scancode) {
    return (key_state[scancode >> 3] >> (scancode & 7)) & 1;
}

void keyboard_update() {
    static int break_next = 0; /* next scancode byte is a key-release */
    static int extended   = 0; /* previous byte was 0xE0 prefix       */

    int data;
    /* Read register ONCE per iteration into a local — avoids double-pop bug */
    while ((data = ps2_ptr[0]) & 0x8000) { /* RVALID = bit 15 */
        unsigned char byte = (unsigned char)(data & 0xFF);

        if (byte == 0xE0) {
            extended = 1;
        } else if (byte == 0xF0) {
            break_next = 1;
        } else {
            unsigned char code = extended ? (byte | 0x80) : byte;
            key_set(code, !break_next);
            break_next = 0;
            extended   = 0;
        }
    }
}

/* ===========================================================================
 * SPRITES
 * =========================================================================*/
#define TRANSPARENT  (short)0xF81F   /* magenta = transparent key colour */
#define PLAYER_W     16
#define PLAYER_H     16
#define TILE_W       16
#define TILE_H       16
#define SPRITE_COUNT 7

typedef enum {
    SPRITE_PLAYER     = 0,
    SPRITE_ENEMY      = 1,
    SPRITE_PROJECTILE = 2,
    SPRITE_TILE_GRASS = 3,
    SPRITE_TILE_DIRT  = 4,
    SPRITE_TILE_STONE = 5,
    SPRITE_TILE_WATER = 6
} SpriteID;

typedef struct {
    int width, height;
    const short *data;
} Sprite;

#define SP (short)
static const short player_sprite[PLAYER_W * PLAYER_H] = {
    SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),
    SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xFFFF),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),SP(0xF81F),
};
#undef SP

static const short grass_sprite[TILE_W * TILE_H] = { [0 ... 255] = (short)0x03E0 };
static const short dirt_sprite [TILE_W * TILE_H] = { [0 ... 255] = (short)0x8400 };
static const short stone_sprite[TILE_W * TILE_H] = { [0 ... 255] = (short)0x7BEF };
static const short water_sprite[TILE_W * TILE_H] = { [0 ... 255] = (short)0x001F };

Sprite sprites[SPRITE_COUNT] = {
    [SPRITE_PLAYER]     = {PLAYER_W, PLAYER_H, player_sprite},
    [SPRITE_ENEMY]      = {PLAYER_W, PLAYER_H, player_sprite},  /* placeholder */
    [SPRITE_PROJECTILE] = {4,        4,        player_sprite},  /* placeholder */
    [SPRITE_TILE_GRASS] = {TILE_W,   TILE_H,   grass_sprite},
    [SPRITE_TILE_DIRT]  = {TILE_W,   TILE_H,   dirt_sprite},
    [SPRITE_TILE_STONE] = {TILE_W,   TILE_H,   stone_sprite},
    [SPRITE_TILE_WATER] = {TILE_W,   TILE_H,   water_sprite},
};

/* ===========================================================================
 * MAP
 * =========================================================================*/
#define MAP_WIDTH  20
#define MAP_HEIGHT 15

static unsigned char current_map[MAP_HEIGHT][MAP_WIDTH];

static const unsigned char map_1[MAP_HEIGHT][MAP_WIDTH] = {
    {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,6,6,6,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,6,6,6,6,6,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,6,6,6,3,3,3,4,4,3,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,4,4,4,4,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,4,4,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,5},
    {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5}
};

static const unsigned char map_2[MAP_HEIGHT][MAP_WIDTH] = {
    {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
    {5,4,4,4,4,5,4,4,4,4,4,4,4,4,5,4,4,4,4,5},
    {5,4,4,4,4,5,4,4,4,4,4,4,4,4,5,4,4,4,4,5},
    {5,4,4,4,4,5,4,4,5,5,5,5,4,4,5,4,4,4,4,5},
    {5,5,4,5,5,5,4,4,5,5,5,5,4,4,5,5,5,4,5,5},
    {5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5},
    {5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5},
    {5,5,5,5,5,5,4,4,5,5,5,5,4,4,5,5,5,5,5,5},
    {5,4,4,4,4,5,4,4,5,5,5,5,4,4,5,4,4,4,4,5},
    {5,4,4,4,4,5,4,4,4,4,4,4,4,4,5,4,4,4,4,5},
    {5,4,4,4,4,5,4,4,4,4,4,4,4,4,5,4,4,4,4,5},
    {5,4,5,5,5,5,5,5,5,4,4,5,5,5,5,5,5,4,5,5},
    {5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5},
    {5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5},
    {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5}
};

void map_init(int map_index) {
    const unsigned char (*src)[MAP_WIDTH] = (map_index == 2) ? map_2 : map_1;
    for (int r = 0; r < MAP_HEIGHT; r++)
        for (int c = 0; c < MAP_WIDTH; c++)
            current_map[r][c] = src[r][c];
}

SpriteID map_get_tile(int row, int col) {
    if (row < 0 || row >= MAP_HEIGHT || col < 0 || col >= MAP_WIDTH)
        return SPRITE_TILE_STONE;
    return (SpriteID)current_map[row][col];
}

void map_set_tile(int row, int col, SpriteID id) {
    if (row >= 0 && row < MAP_HEIGHT && col >= 0 && col < MAP_WIDTH)
        current_map[row][col] = (unsigned char)id;
}

/* ===========================================================================
 * RENDERER
 * =========================================================================*/
void draw_sprite(const Sprite *s, int x, int y) {
    for (int row = 0; row < s->height; row++) {
        int py = y + row;
        if (py < 0 || py >= SCREEN_HEIGHT) continue;
        for (int col = 0; col < s->width; col++) {
            int px = x + col;
            if (px < 0 || px >= SCREEN_WIDTH) continue;
            short colour = s->data[row * s->width + col];
            if ((unsigned short)colour != (unsigned short)TRANSPARENT)
                plot_pixel(px, py, colour);
        }
    }
}

void draw_background() {
    for (int row = 0; row < MAP_HEIGHT; row++)
        for (int col = 0; col < MAP_WIDTH; col++)
            draw_sprite(&sprites[map_get_tile(row, col)], col << 4, row << 4);
}

/* ===========================================================================
 * ENTITY
 * =========================================================================*/
#define MAX_ENTITIES 64
#define HEALTH       100
#define PLAYER_SPEED 2

typedef enum {
    ENTITY_NONE,
    ENTITY_PLAYER,
    ENTITY_ENEMY,
    ENTITY_PROJECTILE
} EntityType;

typedef struct {
    int x, y, dx, dy;
    char facing;
    int width, height;
    int hitbox_offset_x, hitbox_offset_y, hitbox_w, hitbox_h;
    int health;
    int sprite_id;
    short colour;
    EntityType type;
    int active;
} Entity;

Entity entities[MAX_ENTITIES];

/* Forward declarations */
void player_update(Entity *p);
void enemy_update(Entity *e);
void projectile_update(Entity *e);

Entity *spawn_entity(EntityType type) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) {
            entities[i].active = 1;
            entities[i].type   = type;
            return &entities[i];
        }
    }
    return NULL;
}

void draw_entity(Entity *e) {
    draw_sprite(&sprites[e->sprite_id], e->x, e->y);
}

void entity_update_all() {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) continue;
        switch (entities[i].type) {
            case ENTITY_PLAYER:     player_update(&entities[i]);     break;
            case ENTITY_ENEMY:      enemy_update(&entities[i]);      break;
            case ENTITY_PROJECTILE: projectile_update(&entities[i]); break;
            default: break;
        }
    }
}

void entity_draw_all() {
    for (int i = 0; i < MAX_ENTITIES; i++)
        if (entities[i].active) draw_entity(&entities[i]);
}

/* ===========================================================================
 * PLAYER
 * =========================================================================*/
void player_init(Entity *p, SpriteID sprite, short colour) {
    p->x = SCREEN_WIDTH  / 2;
    p->y = SCREEN_HEIGHT / 2;
    p->width  = PLAYER_W;
    p->height = PLAYER_H;
    p->dx = 0; p->dy = 0;
    p->health = HEALTH;
    p->facing = 'n';
    p->hitbox_offset_x = 0; p->hitbox_offset_y = 0;
    p->hitbox_w = p->width; p->hitbox_h = p->height;
    p->sprite_id = (int)sprite;
    p->colour    = colour;
    p->type      = ENTITY_PLAYER;
    p->active    = 1;
}

void player_update(Entity *p) {
    p->dx = 0; p->dy = 0;

    if (key_pressed(KEY_W) || key_pressed(KEY_UP))    { p->dy = -PLAYER_SPEED; p->facing = 'n'; }
    if (key_pressed(KEY_S) || key_pressed(KEY_DOWN))  { p->dy =  PLAYER_SPEED; p->facing = 's'; }
    if (key_pressed(KEY_A) || key_pressed(KEY_LEFT))  { p->dx = -PLAYER_SPEED; p->facing = 'w'; }
    if (key_pressed(KEY_D) || key_pressed(KEY_RIGHT)) { p->dx =  PLAYER_SPEED; p->facing = 'e'; }

    p->x += p->dx;
    p->y += p->dy;

    if (p->x < 0)                         p->x = 0;
    if (p->y < 0)                         p->y = 0;
    if (p->x + p->width  > SCREEN_WIDTH)  p->x = SCREEN_WIDTH  - p->width;
    if (p->y + p->height > SCREEN_HEIGHT) p->y = SCREEN_HEIGHT - p->height;
}

/* ===========================================================================
 * ENEMY / PROJECTILE — stubs (not yet implemented)
 * =========================================================================*/
void enemy_update(Entity *e)      { (void)e; }
void projectile_update(Entity *e) { (void)e; }

/* ===========================================================================
 * GAME
 * =========================================================================*/
void game_init() {
    map_init(1);
    player_init(&entities[0], SPRITE_PLAYER, (short)0xDC14);
}

void update_game() {
    keyboard_update();
    entity_update_all();
}

void draw_game() {
    draw_background();
    entity_draw_all();
}

/* ===========================================================================
 * MAIN
 * =========================================================================*/
int main(void) {
    vga_init();
    timer_init();
    game_init();

    asm volatile("csrw mtvec, %0" : : "r"(exception_handler));
    asm volatile("csrs mstatus, %0" : : "r"(0x8)); /* set MIE bit */

    while (1) {
        if (frame_flag) {
            frame_flag = 0;
            update_game();
            draw_game();
            wait_for_vsync();
        }
    }

    return 0;
}