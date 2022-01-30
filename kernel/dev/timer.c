#include "../kernel.h"

//
// TIMER
//
//

extern void timerirq();

volatile int clock = 0;
volatile int ticks = 0;

int getTicks(){
	Process *p = getCurrentProcess();
	return p->ticks;
}

void resetTicks(){
	Process *p = getCurrentProcess();
	p->ticks = 0;
}

void sleep(int ms){
	Process *p = getCurrentProcess();
	p->timer = 0;
	again:
	if(p->timer < ms) {
		goto again;
	}
}

void irq_timer(Register *r){
	Process *p = getCurrentProcess();
	p->timer++;
	handle_window_manager_interrupt(r);
	outportb(0x20,0x20);
	if(p->timer % 18 == 0){
		p->ticks++;
	}
}

void init_timer(){
	int divisor = 1193180 / 100;       /* Calculate our divisor */
	outportb(0x43, 0x36);             /* Set our command byte 0x36 */
	outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
	outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
	setNormalInt(0,(unsigned long)timerirq);
}

