#include "kernel.h"

void kernel_main(){
	init_video();
	printstring("Welcome to the Sanderslando Kernel!!\n");
	printstring("Loading core components...\n");
	printstring("=> Global Description Table...\n");
	init_gdt();
	printstring("=> Interrupt Description Table...\n");
	init_idt();
	printstring("Loading utilities...\n");
	printstring("=> Programmable Interrupt Timer...\n");
	init_timer();
	printstring("=> PS2...\n");
	init_ps2();
	printstring("=> PCI...\n");
	init_pci();
	printstring("=> Serial ports...\n");
	init_serial();
	printf("Shashwat %d sss %s",1, "test2");
	printstring("\nEnd of loading system!\n");
	char *filesystemtext = dir("@");
	printf("All available drivers: %s \n",filesystemtext);
	filesystemtext = dir("A@");
	printf("All available bootdevices: %s \n",filesystemtext);
	unsigned char* buffer = (unsigned char*)0x2000;
	fread("A@fasm.",buffer);
	if(iself(buffer)){
		printf("ELF: program is ELF!\n");
		unsigned long gamma = loadelf(buffer);
		if(gamma==0){
			printf("ELF: Unable to load ELF!\n");
		}else{
			void* (*foo)() = (void*) gamma;
			foo();
		}
	}else{
		asm volatile ("call 0x2000");
	}
	for(;;);
}

