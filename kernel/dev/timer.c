#include "../kernel.h"

//
// TIMER
//
//

extern void timerirq();

volatile int clock = 0;
volatile int ticks = 0;

int getTicks(){
	return ticks;
}

void resetTicks(){
	ticks = 0;
}

void sleep(int ms){
	clock=0;
	again:
	if(clock<ms){
		goto again;
	}
}

void irq_timer(){
	clock++;
	outportb(0x20,0x20);
	if(clock % 18 == 0){
		ticks++;
	}
}

void init_timer(){
	int divisor = 1193180 / 100;       /* Calculate our divisor */
    	outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    	outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    	outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
    	setNormalInt(0,(unsigned long)timerirq);
}

