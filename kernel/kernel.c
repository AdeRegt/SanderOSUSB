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
	filesystemtext = dir("A@programs");
	printf("All available bootdevices: %s \n",filesystemtext);
	fread("A@programs/voorbeel.",(unsigned char*)0x2000);
	asm volatile ("call 0x2000");
	printf("We are happy to announce the following filesystems: %s\n",filesystemtext);
	for(;;);
}

