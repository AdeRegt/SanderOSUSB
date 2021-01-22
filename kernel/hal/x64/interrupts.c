#include "../../kernel.h"


void init_idt(){
    	/* Sets the special IDT pointer up, just like in 'gdt.c' */
    	idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    	idtp.base = (unsigned long)&idt;

	outportb(0x20, 0x11);
	outportb(0xA0, 0x11);
	outportb(0x21, 0x20);
	outportb(0xA1, 0x28);
	outportb(0x21, 0x04);
	outportb(0xA1, 0x02);
	outportb(0x21, 0x01);
	outportb(0xA1, 0x01);
	outportb(0x21, 0x0);
	outportb(0xA1, 0x0);

    	/* Add any new ISRs to the IDT here using idt_set_gate */
	for(int i = 0 ; i < 32 ; i++){
		idt_set_gate(i, (unsigned)isr_common_stub, 0x08, 0x8E);
	}
    	/* Add any new ISRs to the IDT here using idt_set_gate */
	for(int i = 0 ; i < 32 ; i++){
		idt_set_gate(32+i, (unsigned)irq_common_stub, 0x08, 0x8E);
	}
    	/* Points the processor's internal register to the new IDT */
	idt_set_gate(0x80, (unsigned)isr_special_stub, 0x08, 0x8E);
    	idt_load();
    	asm("sti");
}