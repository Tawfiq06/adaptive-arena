/*This is file is the combination of all the other files into 1 .c*/
/*This is used to test the code on CPulator*/
/*
 * adaptive-arena — flattened single file for CPulator
 * Target: DE1-SoC (RISC-V), VGA 320x240, 60 FPS timer
 */

/*
 * ===========================================================================
 * EXCEPTION / INTERRUPT ENTRY POINT
 * CPUlator jumps here on every interrupt/exception.
 * Must live in the .exceptions section (placed at 0x00000000 by linker).
 *
 * RISC-V does NOT save registers on interrupt entry — we must do it manually.
 * __attribute__((naked)) prevents the compiler emitting any prologue/epilogue.
 * We save/restore all caller-saved regs (ra, t0-t6, a0-a7) so the interrupted
 * code is completely unaffected.
 * ===========================================================================
 */
void __attribute__((section(".exceptions"), naked)) exception_handler(void) {
    asm volatile(
        /* save all caller-saved registers onto the stack */
        "addi sp, sp, -64\n"
        "sw   ra,  0(sp)\n"
        "sw   t0,  4(sp)\n"
        "sw   t1,  8(sp)\n"
        "sw   t2, 12(sp)\n"
        "sw   a0, 16(sp)\n"
        "sw   a1, 20(sp)\n"
        "sw   a2, 24(sp)\n"
        "sw   a3, 28(sp)\n"
        "sw   a4, 32(sp)\n"
        "sw   a5, 36(sp)\n"
        "sw   a6, 40(sp)\n"
        "sw   a7, 44(sp)\n"
        "sw   t3, 48(sp)\n"
        "sw   t4, 52(sp)\n"
        "sw   t5, 56(sp)\n"
        "sw   t6, 60(sp)\n"

        /* read mcause; bit 31 set means it is an interrupt */
        "csrr t0, mcause\n"
        "bgez t0, 1f\n"

        /* mask off interrupt bit to get IRQ number */
        "slli t0, t0, 1\n"
        "srli t0, t0, 1\n"
        "li   t1, 16\n"
        "bne  t0, t1, 1f\n"

        /* timer IRQ: clear TO bit */
        "lui  t2, 0xFF202\n"
        "li   t3, 1\n"
        "sw   t3, 0(t2)\n"

        /* set frame_flag = 1 */
        "la   t2, frame_flag\n"
        "sw   t3, 0(t2)\n"

        "1:\n"
        /* restore all caller-saved registers */
        "lw   ra,  0(sp)\n"
        "lw   t0,  4(sp)\n"
        "lw   t1,  8(sp)\n"
        "lw   t2, 12(sp)\n"
        "lw   a0, 16(sp)\n"
        "lw   a1, 20(sp)\n"
        "lw   a2, 24(sp)\n"
        "lw   a3, 28(sp)\n"
        "lw   a4, 32(sp)\n"
        "lw   a5, 36(sp)\n"
        "lw   a6, 40(sp)\n"
        "lw   a7, 44(sp)\n"
        "lw   t3, 48(sp)\n"
        "lw   t4, 52(sp)\n"
        "lw   t5, 56(sp)\n"
        "lw   t6, 60(sp)\n"
        "addi sp, sp, 64\n"
        "mret\n"
    );
}

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* ===========================================================================
 * ADDRESS MAP
 * =========================================================================*/
#define FPGA_PIXEL_BUF_BASE     0x08000000
#define PIXEL_BUF_CTRL_BASE     0xFF203020
#define TIMER_BASE              0xFF202000
#define PS2_BASE                0xFF200100

/* ===========================================================================
 * VGA
 * =========================================================================*/
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define BUFFER_WIDTH  512

volatile int pixel_buffer_start;
short int Buffer1[SCREEN_HEIGHT][BUFFER_WIDTH];
short int Buffer2[SCREEN_HEIGHT][BUFFER_WIDTH];

static void _swap(int* a, int* b) { int t = *a; *a = *b; *b = t; }

void wait_for_vsync() {
    volatile int *pixel_ctrl_ptr = (volatile int *)PIXEL_BUF_CTRL_BASE;
    *pixel_ctrl_ptr = 1;
    while (*(pixel_ctrl_ptr + 3) & 0x1);
	pixel_buffer_start = *(pixel_ctrl_ptr + 1);
}

void vga_init() {
    volatile int *pixel_ctrl_ptr = (volatile int *)PIXEL_BUF_CTRL_BASE;
    *pixel_ctrl_ptr = 0;
    *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    // clear front
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = 0;
    }
    *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    // clear back
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = 0;
    }
}

void plot_pixel(int x, int y, short int colour) {
    volatile short int *addr =
        (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *addr = colour;
}

void clear_screen() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = 0x0000;
    }
}

void fill_screen(short colour) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        volatile short *row = (volatile short *)(pixel_buffer_start + (y << 10));
        for (int x = 0; x < SCREEN_WIDTH; x++) row[x] = colour;
    }
}

void draw_rect(int x, int y, int width, int height, short colour) {
    int x1 = x + width, y1 = y + height;
    if (x  < 0)            x  = 0;
    if (y  < 0)            y  = 0;
    if (x1 > SCREEN_WIDTH) x1 = SCREEN_WIDTH;
    if (y1 > SCREEN_HEIGHT)y1 = SCREEN_HEIGHT;
    for (int row = y; row < y1; row++) {
        volatile short *rp = (volatile short *)(pixel_buffer_start + (row << 10));
        for (int col = x; col < x1; col++) rp[col] = colour;
    }
}

void draw_line(int x0, int y0, int x1, int y1, short int colour) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)  { _swap(&x0,&y0); _swap(&x1,&y1); }
    if (x0 > x1){ _swap(&x0,&x1); _swap(&y0,&y1); }
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
    timer_ptr[0] = 1;          // clear TO
    timer_ptr[1] = 0x8;        // stop
    timer_ptr[2] = period & 0xFFFF;
    timer_ptr[3] = period >> 16;
    asm volatile("csrs mie, %0" : : "r"(0x10000)); // enable IRQ16
    timer_ptr[1] = 0b111;      // START | CONT | ITO
}

void timer_irq() {
    timer_ptr[0] = 1;
    frame_flag = 1;
}

/* ===========================================================================
 * SPRITES
 * =========================================================================*/
#define TRANSPARENT  0xF81F   // magenta = transparent key
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
    int width;
    int height;
    const short *data;
} Sprite;

/* --- Sprite pixel data --- */
/* short casts silence overflow warnings for 16-bit colour values */
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

static const short grass_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x03E0
};
static const short dirt_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x8400
};
static const short stone_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x7BEF
};
static const short water_sprite[TILE_W * TILE_H] = {
    [0 ... 255] = (short)0x001F
};

Sprite sprites[SPRITE_COUNT] = {
    [SPRITE_PLAYER]     = {PLAYER_W, PLAYER_H, player_sprite},
    [SPRITE_ENEMY]      = {PLAYER_W, PLAYER_H, player_sprite},  // placeholder
    [SPRITE_PROJECTILE] = {4,        4,        player_sprite},  // placeholder
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
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active)
            draw_entity(&entities[i]);
    }
}

/* ===========================================================================
 * PLAYER
 * =========================================================================*/
void player_init(Entity *p, SpriteID sprite, short colour) {
    p->x = SCREEN_WIDTH  / 2;
    p->y = SCREEN_HEIGHT / 2;
    p->width  = PLAYER_W;
    p->height = PLAYER_H;
    p->dx = 0;
    p->dy = 0;
    p->health = HEALTH;
    p->facing = 'n';
    p->hitbox_offset_x = 0;
    p->hitbox_offset_y = 0;
    p->hitbox_w  = p->width;
    p->hitbox_h  = p->height;
    p->sprite_id = (int)sprite;
    p->colour    = colour;
    p->type      = ENTITY_PLAYER;
    p->active    = 1;
}

void player_update(Entity *p) {
    p->x += p->dx;
    p->y += p->dy;
    if (p->x < 0)                       p->x = 0;
    if (p->y < 0)                       p->y = 0;
    if (p->x + p->width  > SCREEN_WIDTH)  p->x = SCREEN_WIDTH  - p->width;
    if (p->y + p->height > SCREEN_HEIGHT) p->y = SCREEN_HEIGHT - p->height;
}

void player_draw(const Entity *p) {
    draw_rect(p->x, p->y, p->width, p->height, p->colour);
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

    /* Point mtvec at our exception handler so the CPU knows where to jump */
    extern void exception_handler(void);
    asm volatile("csrw mtvec, %0" : : "r"(exception_handler));

    /* Enable global interrupts (mstatus.MIE = bit 3) */
    asm volatile("csrs mstatus, %0" : : "r"(0x8));

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