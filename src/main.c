#include "address_map.h"
#include "vga.h"
#include "timer.h"
#include "game.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

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

typedef enum{
    START_SCREEN,
    PLAYER_SELECT,
    MAP_SELECT,
    GAME,
    PAUSE
} MODE;

int main(void){
    int mode = START_SCREEN; //will control if we are start screen etc

    vga_init();
    timer_init();
    game_init();

    asm volatile("csrw mtvec, %0" : : "r"(exception_handler));

    asm volatile("csrs mstatus, %0" : : "r"(0x8));
    int cur_buf = 0;
    while(1){

        if(frame_flag){
            frame_flag = 0;
            update_game(cur_buf);
            draw_game(cur_buf);
            
            wait_for_vsync(); //display frame
            cur_buf = 1 - cur_buf; //to keep track of our current buff
        }
    }

    return 0;
}