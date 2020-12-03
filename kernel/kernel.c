#include "kernel.h"
#include "exec/program.h"

void browser(){
	while(1){
		char *pt = browse();
		unsigned char* buffer = (unsigned char*)0x2000;
		if(fexists((unsigned char *)pt) && fread(pt,buffer)){
			cls();
			char *c[3];
			c[0] = "view";
			c[1] = "run";
			c[2] = "cancel";
			unsigned int t = choose("How can I help you",3,c);
			if(t==0){
				int offset = 0;
				again:
				cls();
				for(int i = 0 ; i < 512 ; i++){
					printf("%c",buffer[offset+i]);
				}
				printf("\n\nup,down,q=quit\n");
				volatile unsigned char res = getch();
				if(res==VK_UP){
					offset -= 512;
				}
				if(res==VK_DOWN){
					offset += 512;
				}
				if(res!='q'){
					goto again;
				}
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
		}else{
			confirm("Could not find file");
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
	printstring("=> Paging...\n");
	init_paging();
	printstring("=> Multitasking...\n");
	init_multitasking();
	printstring("=> PS2...\n");
	init_ps2();
	printstring("=> PCI...\n");
	init_pci();
	printstring("=> Serial ports...\n");
	init_serial();
	printstring("=> APCI...\n");
	init_acpi();
	printstring("=> Soundblaster...\n");
	init_soundblaster16();
	printf("Shashwat %d sss %s",1, "test2");
	printstring("\nEnd of loading system!\n");
	
	if(init_graph_vga(320, 200, 1)==0) {
		printf("VGA: failed to set!\nPress any key to reboot\n");
		getch();
		poweroff();
	}
	
	if(confirm("kernel created by sander de regt, shashwat shagun, johan gericke, daniel mccarthy and pablo narvaja") == 0) {
		poweroff();
	};

	if(!getDeviceCount()) {
		message("Unable to discover usefull hardware");
		poweroff();
	}
	
	browser();

	for(;;);
}

