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
	debugf("\n\n");
	debugf("/===============================================\\\n");
	debugf("| K E R N E L   P A N I C   S T A C K T R A C E |\n");
	debugf("+===============================================+\n");
	debugf("| EIP=%x \n",r->eip);
	debugf("| EAX=%x EBX=%x ECX=%x EDX=%x \n",r->eax,r->ebx,r->ecx,r->edx);
	debugf("+------------------------------------------------\n");
	debugf("| Stacktrace:\n");
    for(int i = 0; i < 5 && stk != NULL; stk = stk->ebp, i++)
    {
        // Unwind to previous stack frame
        printf("[ %x ]> %x \n", i, ((unsigned long)stk->eip) & 0xFFFFFFFF);
		debugf("| --> %x \n",((unsigned long)stk->eip) & 0xFFFFFFFF);
    }
	debugf("+------------------------------------------------\n");
	printf("\n\n System halted \n\n");
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
	unsigned long maxsize;
}FileSymbol;

#define MAX_FILE_SYMBOLS 10
FileSymbol filesymboltable[MAX_FILE_SYMBOLS];
extern volatile char* last_tty_command;

void special_handler(Register *r){
	outportb(0xA0,0x20);
	outportb(0x20,0x20);// printf("OKE: eax=%x \n",r->eax);__asm__ __volatile__("cli\nhlt"); for(;;);
	debugf("INT0x80: EIP=%x \n",r->eip);

	if(r->eax==0x01){ // EXIT
		printf("INT0x80: PROGRAM FINISHED\n");
		debugf("INT0x80: PROGRAM FINISHED\n");
		if(isGraphicsMode()){
			if(r->ebx){
				r->eip = (unsigned long)exit_program_and_wait_for_keypress;
			}else{
				r->eip = (unsigned long)browser;
			}
		}else{
			r->eip = (unsigned long)tty_loop;
		}
	}
	else if(r->eax==0x03){ // F-READ
		debugf("INT0x80: READ\n");
		if(r->ebx==1){ // FROM STDOUT
			volatile unsigned char kt = ((volatile unsigned char*)&keyword)[0];
			((unsigned char*)r->ecx)[0] = kt;
			((volatile unsigned char*)&keyword)[0] = 0;
		}else{ // TO FILE
			if(filesymboltable[r->ebx-3].inuse){
				memset((char*)r->ecx,r->edx,0);
				memcpy((char*)(filesymboltable[r->ebx-3].pointer + filesymboltable[r->ebx-3].ftell) ,(char*)r->ecx,r->edx);
				r->eax = r->edx;
				((char*)r->ecx)[r->edx+0] = 0x00;
				((char*)r->ecx)[r->edx+1] = 0x00;
				((char*)r->ecx)[r->edx+3] = 0x00;
				((char*)r->ecx)[r->edx+4] = 0xCD;
			}else{
				printf("not allowed call\n");for(;;);
			}
		}
		r->eax=r->edx;
	}
	else if(r->eax==0x04){ // F-WRITE
		debugf("INT0x80: WRITE\n");
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
		unsigned char filemode = ((unsigned char*)r->ecx)[0];
		debugf("INT0x80: OPEN [ %s ] with method [ %c ] \n",file,filemode);
		if(filemode!='r'){
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
				unsigned long nieuwetel = 0;
				while(1){
					unsigned char t = ((unsigned char*)(filesymboltable[fileloc-3].pointer))[nieuwetel];
					if(t==0){
						break;
					}
					nieuwetel++;
				}
				filesymboltable[fileloc-3].maxsize = nieuwetel;
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
		debugf("INT0x80: Closing file\n");
		r->eax = 0;
	}
	else if(r->eax==0x4E){ // GET SYSTEMTIME
		debugf("INT0x80: SYSTIME\n");
		r->eax=0;
	}
	else if(r->eax==0xC0){ // MALLOC
		debugf("INT0x80: MALLOC\n");
		unsigned long size = r->ebx;
		r->eax = (unsigned long) malloc(size);
	}
	else if(r->eax==0xC1){ // FREE
		debugf("INT0x80: FREE\n");
		unsigned long location = r->ebx;
		free((void*)location);
	}
	else if(r->eax==0xC2){ // SEEK
		unsigned long location = r->ebx;
		unsigned long offset = r->ecx;
		unsigned long whence = r->edx;
		// printf("About to seek\n");
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
		debugf("INT0x80: SEEK %x %x %x \n",location,offset,whence);//for(;;);
		r->eax = 0;
	}
	else if(r->eax==0xC3){ // TELL
		unsigned long location = r->ebx;
		debugf("INT0x80: TELL %x with %x \n",location,filesymboltable[location-3].ftell);//for(;;);
		r->eax = filesymboltable[location-3].ftell;
	}
	else if(r->eax==0xC4){ // UNGETC
		debugf("INT0x80: UNGETC\n");
		if(filesymboltable[r->ebx-3].inuse){
			filesymboltable[r->ebx-3].ftell++;
			((unsigned char*)(filesymboltable[r->ebx-3].pointer + filesymboltable[r->ebx-3].ftell))[0] = (unsigned char)(r->ecx & 0xFF);
			r->eax = 1;
		}else{
			r->eax = 0;
		}
	}
	else if(r->eax==0xC5){ // GETC
		debugf("INT0x80: GETC\n");
		if(r->ebx==1){
			r->eax = getch();
		}else if(filesymboltable[r->ebx-3].inuse){
			r->eax = 0;
			r->eax = ((unsigned char*)(filesymboltable[r->ebx-3].pointer + filesymboltable[r->ebx-3].ftell))[0] & 0xFF;
			if(filesymboltable[r->ebx-3].ftell>=filesymboltable[r->ebx-3].maxsize){
				r->eax = 0;
			}
			filesymboltable[r->ebx-3].ftell++;
		}else{
			r->eax = 0;
		}
	}
	else if(r->eax==0xC6){ // REALPATH
		debugf("INT0x80: REALPATH\n");
		r->eax = r->ebx;
	}
	else if(r->eax==0xC7){ // REALLOC
		debugf("INT0x80: REALLOC\n");
		r->eax = (unsigned int) realloc((void *)r->ebx,r->ecx);
	}
	else if(r->eax==0xC8){ // ETH::IN
		debugf("INT0x80: GETNETWORKPACKAGE\n");
		PackageRecievedDescriptor prd = getEthernetPackage();
		r->eax = prd.low_buf;
	}
	else if(r->eax==0xC9){ // ETH::OUT
		int type = r->ebx;
		int size = r->ecx;
		unsigned char* addr = (unsigned char*) r->edx;
		unsigned int loca = r->esi;
		unsigned int port = r->edi;
		debugf("INT0x80: SENDNETWORKPACKAGE type=%x size=%x to=%d.%d.%d.%d where=%x port=%x\n",type,size,addr[0],addr[1],addr[2],addr[3],loca,port);

		unsigned char macaddrto[SIZE_OF_MAC];
		unsigned char* m1;
		if(addr[0]==192){ // is local
			m1 = getMACFromIp(addr);
		}else{ // is public
			m1 = getMACFromIp(getOurRouterIp());
		}
		macaddrto[0] = m1[0];
		macaddrto[1] = m1[1];
		macaddrto[2] = m1[2];
		macaddrto[3] = m1[3];
		macaddrto[4] = m1[4];
		macaddrto[5] = m1[5];

		if(type==2){
			
			int calbuffersize = sizeof(struct TCPHeader)+size;
			void *pnt = (void*) malloc(calbuffersize);
			struct TCPHeader *tcp = (struct TCPHeader*) pnt;
			unsigned char* from = (unsigned char*) pnt;
			unsigned char* to = (unsigned char*) loca;
			for(int i = 0 ; i < size ; i++){
				from[sizeof(struct TCPHeader)+i] = to[i];
			}
			to[size] = 0;
			
			fillTcpHeader(tcp,(unsigned char*)&macaddrto,calbuffersize-sizeof(struct EthernetHeader),getOurIpAsLong(),((unsigned long*) r->edx)[0],port,port,0x1010,0x1010,5,TCP_PUS | TCP_ACK,64240);

			unsigned short* tw = (unsigned short*) loca;
			unsigned int z = 0;
			int ct = size/2;
			if(size%2){
				ct++;
			}
			for(int i = 0 ; i < ct ; i++){
				unsigned short qw = switch_endian16(tw[i]);
				z += qw;
			}

			unsigned short checksum_old = switch_endian16(~tcp->checksum);
			unsigned int checksum_comp = checksum_old + z;
			unsigned int checksum = (checksum_comp & 0xffff) + (checksum_comp >> 16);
    		checksum += (checksum >> 16);

    		unsigned short final = ~checksum;

    		unsigned short chsm = switch_endian16(final);
			tcp->checksum = chsm;

			PackageRecievedDescriptor prd;
			prd.buffersize = calbuffersize;
			prd.high_buf = 0;
			prd.low_buf = (unsigned long)pnt;
			sendEthernetPackage(prd);
			
		}else if(type==1){
			
			int calbuffersize = sizeof(struct UDPHeader)+size;
			void *pnt = (void*) malloc(calbuffersize);
			struct UDPHeader *udp = (struct UDPHeader*) pnt;
			fillUdpHeader(udp,(unsigned char*)&macaddrto,calbuffersize-sizeof(struct EthernetHeader),getOurIpAsLong(),((unsigned long*) r->edx)[0],port,port);

			unsigned char* from = (unsigned char*) pnt;
			unsigned char* to = (unsigned char*) loca;
			for(int i = 0 ; i < size ; i++){
				from[sizeof(struct UDPHeader)+i] = to[i];
				debugf("%x ",to[i]);
			}
			debugf("\n");

			PackageRecievedDescriptor prd;
			prd.buffersize = calbuffersize;
			prd.high_buf = 0;
			prd.low_buf = (unsigned long)pnt;
			sendEthernetPackage(prd);
		}

		r->eax = 0;
	}
	else if(r->eax==0xCA){ // ETH::INIT
		int type = r->ebx;
		int is_ip = r->ecx;
		unsigned char* addr = (unsigned char*) r->edx;
		if(is_ip==0){
			debugf("INT0x80: looking for %s \n",addr);
			addr = getIPFromName((char*)addr);
		}
		unsigned int func = r->esi;
		unsigned int port = r->edi;
		debugf("INT0x80: INITNETWORK type=%x is_ip=%x to=%d:%d:%d:%d function=%x port=%x \n",type,is_ip,addr[0],addr[1],addr[2],addr[3],func,port);
		if(type!=2){
			r->eax = 1;
			return;
		}
		create_tcp_session(getOurIpAsLong(),((unsigned long*)addr)[0],port & 0xFFFF,port & 0xFFFF,func);
		r->eax = 0;
	}
	else if(r->eax==0xCB){ // PRGS::ARG
		debugf("INT0x80: GETPARAMS\n");
		int argcount = 0; // we always start with 1, this is the object itself
		unsigned long *args = (unsigned long *)malloc(sizeof(unsigned long)*10);
		args[argcount++] = (unsigned long)"example"; // example -m 10 test test
		if(strlen((char*)last_tty_command)){
			args[argcount++] = (unsigned long) (last_tty_command);
			int z = strlen((char*)last_tty_command);
			for(int i = 0 ; i < z ; i++){
				if(last_tty_command[i]==' '){
					last_tty_command[i] = 0;
					args[argcount++] = (unsigned long) (last_tty_command+i+1);
				}
			}
		}
		for(int i = 0 ; i < argcount ; i++){
			debugf("INT0x80: serving on arg %x the value %s \n",i,(char*)args[i]);
		}
		r->eax = (unsigned long)args; // arguments
		r->ebx = argcount; // argumentcount
	}
	else{
		printf("INT0x80: UNKNOWN SYSCALL %x \n",r->eax);
		debugf("INT0x80: UNKNOWN SYSCALL %x \n",r->eax);
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
