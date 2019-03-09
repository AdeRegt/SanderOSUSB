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
	char *filesystemtext = dir("A@");
	filesystemtext = fread("A@kernel.bin");
	if(filesystemtext[0]==0x00){
		printf("Unable to allocate directory\n");
	}else{
		printf("We are happy to announce the following filesystems: %s\n",filesystemtext);
	}
	for(;;);
}

