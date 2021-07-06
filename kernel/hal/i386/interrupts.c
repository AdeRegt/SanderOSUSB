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

struct stackframe{
	struct stackframe *ebp;
	void *eip;
}__attribute__((packed));

void fault_handler(Register *r){
	asm volatile("cli");
	// handle panick like it should be!
	setForeGroundBackGround(0x7,0x4);
	cls();
	if(isGraphicsMode()){
		printf("K E R N E L   P A N I C \n\n");
		printf("There is a error in the kernel which caus");
		printf("sed the OS to stop!\n");
	}else{
		printf("==============================================\n");
		printf("|         K E R N E L   P A N I C            |\n");
		printf("==============================================\n");
		printf("Something serious happend and this causes the \n");
		printf("Operatingsystem to halt operations. Debuginfo:\n");
	}
	printf("\n");
	printf("EIP=%x \n",r->eip);
	printf("EAX=%x\n",r->eax);
	printf("\n");
	struct stackframe *stk = NULL;
	__asm__("movl %%ebp, %[stk]" :  /* output */ [stk] "=r" (stk));
    // asm volatile ("movl %%ebp,%0" : "r"(stk) ::);
    for(int i = 0; i < 5 && stk != NULL; stk = stk->ebp, i++)
    {
        // Unwind to previous stack frame
        printf("[ %x ]> %x \n", i, ((unsigned long)stk->eip) & 0xFFFFFFFF);
    }
	printf("\n\n System halted \n\n");
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

void exit_program_and_wait_for_keypress(){
	printf("End of program, press any key to return\n");
	getch();
	int mode = 1;
	int status = 0;
	__asm__ __volatile__ ("int $0x80": "+a" (mode) , "+b" (status));
}

typedef struct {
	unsigned long ftell;
	unsigned char inuse;
	void *pointer;
	char filename[100];
}FileSymbol;

#define MAX_FILE_SYMBOLS 10
FileSymbol filesymboltable[MAX_FILE_SYMBOLS];

void special_handler(Register *r){
	outportb(0xA0,0x20);
	outportb(0x20,0x20);//printf("OKE: eax=%x \n",r->eax);__asm__ __volatile__("cli\nhlt"); for(;;);

	if(r->eax==0x01){ // EXIT
    		printf("\nProgram finished!\n");
			if(r->ebx){
				r->eip = (unsigned long)exit_program_and_wait_for_keypress;
			}else{
				r->eip = (unsigned long)browser;
			}
	}
	else if(r->eax==0x03){ // F-READ
		printf("INT0x80: READ\n");
		if(r->ebx==1){ // FROM STDOUT
			volatile unsigned char kt = ((volatile unsigned char*)&keyword)[0];
			((unsigned char*)r->ecx)[0] = kt;
			((volatile unsigned char*)&keyword)[0] = 0;
		}else{ // TO FILE
			if(filesymboltable[r->ebx-3].inuse){
				memset((char*)r->ecx,r->edx,0);
				memcpy((char*)filesymboltable[r->ebx-3].pointer,(char*)r->ecx,r->edx);
				r->eax = r->edx;
				((char*)r->ecx)[r->edx+0] = 0x00;
			}else{
				printf("not allowed call\n");for(;;);
			}
		}
		r->eax=r->edx;
	}
	else if(r->eax==0x04){ // F-WRITE
		if(r->ebx==1||r->ebx==2){ // TO STDOUT OR STDERR
			const char *buf = (const char *)r->ecx;
			for(unsigned long i = 0 ; i < r->edx ; i++){
				putc(buf[i]);
			}
		}else{ // TO FILE
			r->eax = fwrite((char *)filesymboltable[r->ebx-3].filename,(unsigned char *)r->ecx,r->edx);
			// printf("INT0x80: unknown write (%x)\n",r->ebx);for(;;);
		}
		r->eax = (signed int)0;//r->eax=r->edx;
		// printf("OKE: eax=%x , edx=%x , ecx=%x , ebx=%x \n",r->eax,r->edx,r->ecx,r->ebx);__asm__ __volatile__("cli\nhlt"); for(;;);
	}
	else if(r->eax==0x05){ // OPEN FILE
		unsigned char* file = ((unsigned char*)r->ebx);
		printf("INT0x80: OPEN %s \n",file);
		if(((unsigned char*)r->ecx)[0]=='w'){
			char* file2 = ((char*)r->ebx);
			int fileloc = 3;
			int z = 0;
			for(z = 0 ; z < MAX_FILE_SYMBOLS ; z++){
				if(filesymboltable[z].inuse==0){
					filesymboltable[z].inuse = 1;
					fileloc = 3+z;
					break;
				}
			}
			filesymboltable[fileloc-3].ftell = 0;
			memcpy(file2,filesymboltable[fileloc-3].filename,strlen(file2));
			r->eax = fileloc;
		}else{
			char* file2 = ((char*)r->ebx);
			if(fexists(file)){
				int fileloc = 3;
				int z = 0;
				for(z = 0 ; z < MAX_FILE_SYMBOLS ; z++){
					if(filesymboltable[z].inuse==0){
						filesymboltable[z].inuse = 1;
						fileloc = 3+z;
						break;
					}
				}
				filesymboltable[fileloc-3].ftell = 0;
				void *dataset = malloc(0x100);
				fread(file2,dataset);
				filesymboltable[fileloc-3].pointer = dataset;
				memcpy(file2,filesymboltable[fileloc-3].filename,strlen(file2));
				r->eax = fileloc;
			}else{
				r->eax = 0;
			}
		}
	}
	else if(r->eax==0x06){ // CLOSE FILE
		unsigned int file = r->ebx;
		filesymboltable[file-3].inuse = 0;
		free(filesymboltable[file-3].pointer);
		printf("INT0x80: Closing file\n");
		r->eax = 0;
	}
	else if(r->eax==0x4E){ // GET SYSTEMTIME
		printf("INT0x80: SYSTIME\n");
		r->eax=0;
	}
	else if(r->eax==0xC0){ // MALLOC
		printf("INT0x80: MALLOC\n");
		unsigned long size = r->ebx;
		r->eax = (unsigned long) malloc(size);
	}
	else if(r->eax==0xC1){ // FREE
		printf("INT0x80: FREE\n");
		unsigned long location = r->ebx;
		free((void*)location);
	}
	else if(r->eax==0xC2){ // SEEK
		unsigned long location = r->ebx;
		unsigned long offset = r->ecx;
		unsigned long whence = r->edx;
		printf("About to seek\n");
		if(whence==0){
			filesymboltable[location-3].ftell = offset;
		}else if(whence==1){
			filesymboltable[location-3].ftell += offset;
		}else if(whence==2){
			int nieuwetel = filesymboltable[location-3].ftell;
			while(1){
				unsigned char t = ((unsigned char*)(filesymboltable[location-3].pointer))[nieuwetel];
				if(t==0){
					break;
				}
				nieuwetel++;
			}
			filesymboltable[location-3].ftell = nieuwetel;
		}
		printf("INT0x80: SEEK %x %x %x \n",location,offset,whence);//for(;;);
		r->eax = 0;
	}
	else if(r->eax==0xC3){ // TELL
		unsigned long location = r->ebx;
		printf("INT0x80: TELL %x with %x \n",location,filesymboltable[location-3].ftell);//for(;;);
		r->eax = filesymboltable[location-3].ftell;
	}
	else{
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
