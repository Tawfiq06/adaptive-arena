#include "timer.h"
#include "address_map.h"

#define FPS 60
#define TIMER_FREQ 100000000

volatile int frame_flag = 0;

static volatile unsigned int *timer_ptr = (unsigned int*)TIMER_BASE;

void timer_init(){
    // 60 - FPS -> 100MHz / 60
    unsigned int period =  TIMER_FREQ / FPS;
    timer_ptr[0] = 1; // Clear TO
    
    timer_ptr[1] = 0x8; // stop timer

    //load low bits
    timer_ptr[2] = period;
    
    //load high bits
    timer_ptr[3] = (period >> 16);

    asm volatile("csrs mie, %0" : : "r"(0x10000)); //enable IRQ16

    // turn on  CONT START and IRQ
    timer_ptr[1] = 0b111;
}

void timer_irq(){
   timer_ptr[0] = 1;
   frame_flag = 1;
}