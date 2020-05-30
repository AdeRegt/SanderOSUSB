#include "../../kernel.h"
//
// Interrupt Descriptor Table
//
//

/* Defines an IDT entry */
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;        /* Our kernel segment goes here! */
    unsigned char always0;     /* This will ALWAYS be set to 0! */
    unsigned char flags;       /* Set using the above table! */
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

/* This exists in 'start.asm', and is used to load our IDT */
extern void idt_load();

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags){
    /* The interrupt routine's base address */
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void setErrorInt(unsigned char num,unsigned long base){
	idt_set_gate(num, base, 0x08, 0x8E);
}

void setNormalInt(unsigned char num,unsigned long base){
	idt_set_gate(32+num, base, 0x08, 0x8E);
}


extern void isr_common_stub();
extern void irq_common_stub();
extern void isr_special_stub();

void fault_handler(){
	printf("\n\n -= KERNEL PANIC =- \n\n");
	asm volatile("cli");
	asm volatile("hlt");
}


void irq_handler(){
	outportb(0x20, 0x20);
	outportb(0xA0, 0x20);
}

//
// EAX
extern char keyword;
extern void browser();

void special_handler(Register *r){
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
	if(r->eax==0x01){ // EXIT
    		printf("\nProgram finished!");
			r->eip = (unsigned long)browser;
	}else if(r->eax==0x03){ // F-READ
		if(r->ebx==1){ // FROM STDOUT
			volatile unsigned char kt = ((volatile unsigned char*)&keyword)[0];
			((unsigned char*)r->ecx)[0] = kt;
			((volatile unsigned char*)&keyword)[0] = 0;
		}else{ // TO FILE
			printf("INT0x80: unknown read (%x)\n",r->ebx);
		}
		r->eax=r->edx;
	}else if(r->eax==0x04){ // F-WRITE
		if(r->ebx==1){ // TO STDOUT
			for(unsigned int i = 0 ; i < r->edx ; i++){
				printf("%c",((unsigned char*)r->ecx)[i]);
			}
		}else{ // TO FILE
			printf("INT0x80: unknown write (%x)\n",r->ebx);
		}
		r->eax=r->edx;
	}else if(r->eax==0x05){ // OPEN FILE
		unsigned char* file = ((unsigned char*)r->ebx);
		printf("INT0x80: Looking for %s \n",file);
		if(fexists(file)){
			printf("INT0x80: file exists\n");
		}else{
			printf("INT0x80: file NOT exists\n");
		}
	}else if(r->eax==0x4E){ // GET SYSTEMTIME
		r->eax=0;
		printf("INT0x80: asked for systemtime\n");
	}else{
		printf("INT0x80: UNKNOWN SYSCALL %x \n",r->eax);
		for(;;);
	}
}

/* Installs the IDT */
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

//
// Global Description Table
//
//

struct gdt_entry{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran){
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void init_gdt(){
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (unsigned long)&gdt;
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_flush();
}
