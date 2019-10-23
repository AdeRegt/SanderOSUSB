#include "kernel.h"

void browser(){
	while(1){
			char *pt = browse();
			if(fexists((unsigned char *)pt)){
				unsigned char* buffer = (unsigned char*)0x2000;
				fread(pt,buffer);
				cls();
				printf("Open Execute or Cancel\n\n");
				char x = getch();
				if(x=='o'){
					for(int i = 0 ; i < 512 ; i++){
						printf("%c",buffer[i]);
					}
					printf("\n\nPress any key to continue\n");
					getch();
				}else if(x=='e'){
					if(iself(buffer)){
						printf("ELF: program is ELF!\n");
						unsigned long gamma = loadelf(buffer);
						if(gamma==0){
							printf("ELF: Unable to load ELF!\n");
						}else{
							cls();
							void* (*foo)() = (void*) gamma;
							foo();
						}
						printf("ELF: Waiting\n");
						getch();
					}else{
						cls();
						asm volatile ("call 0x2000");
					}
				}
			}
		}
}

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
	printstring("=> APCI...\n");
	init_acpi();
	printf("Shashwat %d sss %s",1, "test2");
	printstring("\nEnd of loading system!\n");
	//320,200
	if(init_graph_vga(320,200,1)==0){
		printf("VGA: failed to set!\n");
		for(;;);
	}
	if(confirm("kernel created by sander de regt and shashwat shagun and johan gericke")==0){
		init_acpi();
		poweroff();
	}
	if(getDeviceCount()){
		browser();
	}else{
		message("Unable to discover usefull hardware");
		init_acpi();
		poweroff();
	}
	for(;;);
}

