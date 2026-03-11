#ifndef TIMER_H
#define TIMER_H

extern volatile int frame_flag;

void timer_init();
void timer_irq();

#endif