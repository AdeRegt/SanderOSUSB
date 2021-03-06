#include "../../kernel.h"

typedef struct {
    Register registers;
    unsigned char active;
}Process;

extern void timerirq();

Process registers[10];
static int regnow = 0;
static int regcount = 0;

void irq_multitasking(Register *r){
    irq_timer();
    asm volatile ("cli");
    if(regcount>0){

        //
        // first copy all registers to stack
        registers[regnow].registers.gs         =   r->gs;
        registers[regnow].registers.fs         =   r->fs;
        registers[regnow].registers.es         =   r->es;
        registers[regnow].registers.ds         =   r->ds;
        registers[regnow].registers.edi        =   r->edi;
        registers[regnow].registers.esi        =   r->esi;
        registers[regnow].registers.ebp        =   r->ebp;
        registers[regnow].registers.esp        =   r->esp;
        registers[regnow].registers.ebx        =   r->ebx;
        registers[regnow].registers.edx        =   r->edx;
        registers[regnow].registers.ecx        =   r->ecx;
        registers[regnow].registers.eax        =   r->eax;
        registers[regnow].registers.eip        =   r->eip;
        registers[regnow].registers.cs         =   r->cs;
        registers[regnow].registers.eflags     =   r->eflags;
        registers[regnow].registers.useresp    =   r->useresp;
        registers[regnow].registers.ss         =   r->ss;

        while(1){
            regnow++;
            if(!(regnow<regcount)){
                regnow = 0;
            }
            if(registers[regnow].active){
                break;
            }
        }

        //
        // then pop all registers from stack
        r->gs       =       registers[regnow].registers.gs;
        r->fs       =       registers[regnow].registers.fs;
        r->es       =       registers[regnow].registers.es;
        r->ds       =       registers[regnow].registers.ds;
        r->edi      =       registers[regnow].registers.edi;
        r->esi      =       registers[regnow].registers.esi;
        r->ebp      =       registers[regnow].registers.ebp;
        r->esp      =       registers[regnow].registers.esp;
        r->ebx      =       registers[regnow].registers.ebx;
        r->edx      =       registers[regnow].registers.edx;
        r->ecx      =       registers[regnow].registers.ecx;
        r->eax      =       registers[regnow].registers.eax;
        r->eip      =       registers[regnow].registers.eip;
        r->cs       =       registers[regnow].registers.cs;
        r->eflags   =       registers[regnow].registers.eflags;
        r->useresp  =       registers[regnow].registers.useresp;
        r->ss       =       registers[regnow].registers.ss;


    }
    asm volatile ("sti");
    return;
}

int activeTask(){
    return regnow;
}

void killTask(int pid){
    registers[pid].active = 0;
}

int createTask(unsigned long entrypoint){
    asm volatile ("cli");
    registers[regcount].registers.cs = 0x08;
    registers[regcount].registers.ds = 0x10;
    registers[regcount].registers.ss = 0x10;
    registers[regcount].registers.eip = entrypoint;
    registers[regcount].active = 1;
    asm volatile ("sti");
    int cup = regcount;
    regcount++;
    return cup;
}

void init_multitasking(){
    regcount = 1;
    printf("[MULT] Multitasking module enabled!\n");
    setNormalInt(0,(unsigned long)timerirq);
    registers[0].active = 1;
    printf("[MULT] Finished multitasking\n");
}