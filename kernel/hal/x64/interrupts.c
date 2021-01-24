#include "../../kernel.h"

struct IDTPointer{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct IDTTableElement{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero_1;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint8_t zero_2;
} __attribute__ ((packed));

struct IDTPointer idtpointer;
struct IDTTableElement idttable[MAX_INTERRUPTS];

extern void isr_common_stub();

void panic_isr(){
    outportb(0x20, 0x20);
	outportb(0xA0, 0x20);
    printf("KERNELPANIC");
    asm volatile("cli\nhlt");
}


void init_idt(){

    //
    // Disable interrupts
    asm("cli");
    
    //
    // Setup base pointer
    idtpointer.limit = (uint16_t)(sizeof(struct IDTTableElement) * MAX_INTERRUPTS) - 1;
    idtpointer.base = (uint64_t)&idttable;

    //
    // Fill with default interrupts
    unsigned short s = getCSValue();
    for(int i = 0 ; i < MAX_INTERRUPTS ; i++){
		idt_set_gate(i, (pointer)isr_common_stub, s, 0x8E);
	}

    //
    // Load the new table
    __asm__ __volatile__ ("lidt %0" : : "m" (idtpointer));

    //
    // Re-enable interrupts
    asm("sti");
    
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags){
    idttable[num].offset_low    = (base & 0xFFFF);
    idttable[num].selector      = getCSValue();
    idttable[num].zero_1        = 0;
    idttable[num].attributes    = flags;
    idttable[num].offset_middle = (base >> 16) & 0xFFFF;
    idttable[num].offset_high   = (base >> 32) & 0xFFFFFFFF;
    idttable[num].zero_2        = 0;
}

void setErrorInt(unsigned char num,unsigned long base){
    idt_set_gate(num, base, 0x08, 0x8E);
}

void setNormalInt(unsigned char num,unsigned long base){
    idt_set_gate(OFFSET_NORMAL_INT+num, base, 0x08, 0x8E);
}