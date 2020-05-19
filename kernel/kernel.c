#include "kernel.h"
#include "exec/program.h"

void browser(){
	while(1){
			char *pt = browse();
			if(fexists((unsigned char *)pt)){
				unsigned char* buffer = (unsigned char*)0x2000;
				fread(pt,buffer);
				cls();
				char *c[3];
				c[0] = "view";
				c[1] = "run";
				c[2] = "cancel";
				unsigned int t = choose("How can I help you",3,c);
				if(t==0){
					for(int i = 0 ; i < 512 ; i++){
						printf("%c",buffer[i]);
					}
					printf("\n\nPress any key to continue\n");
					getch();
				}else if(t==1){
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
					}else{
						cls();
						void* (*foo)() = (void*) 0x2000;
						foo();
					}
					printf("\n - Program finished! Press any key to return - \n");
					getch();
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


	if(!getDeviceCount())
	{
		message("Unable to discover usefull hardware");
		init_acpi();
		poweroff();
	}
	
	browser();

	for(;;);
}

