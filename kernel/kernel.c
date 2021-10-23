#include "kernel.h"
#include "exec/program.h"

void no_usefull_hardware(){
	message("Unable to discover usefull hardware");
	poweroff();
	for(;;);
}

void browser(){
	while(1){
		char *pt = browse();
		if(pt==0){
			no_usefull_hardware();
		}
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
					debugf("ELF: program is ELF!\n");
					unsigned long gamma = loadelf(buffer);
					if(gamma==0){
						debugf("ELF: Unable to load ELF!\n");
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

GRUBStatus grubstatus;
GRUBStatus getGrubStatus(){
	return grubstatus;
}

void kernel_main(GRUBMultiboot *grub, unsigned long magic){
	init_video();
	printstring("Welcome to the Sanderslando Kernel!!\n");
	printstring("=> Global Description Table...\n");
	init_gdt();
	printstring("=> Interrupt Description Table...\n");
	init_idt();
	cpuid_get_details();
	printstring("Loading core components...\n");
	if(magic==0x2BADB002){
		debugf("[GRUB] Multiboot compliant bootloader!\n");
		unsigned char* cmdline = (unsigned char*) grub->cmdline;
		debugf("[GRUB] CMDLINE \"%s\" \n",cmdline);
		unsigned char token[50];
		int got = 0;
		again:
			for(int i = 0 ; i < 50 ; i++){
				token[i] = 0x00;
			}
			int z = 0;
			unsigned char t = 0x00;
			while(1){
				t = cmdline[got];
				if(t==0x00){
					break;
				}
				if(t==' '){
					got++;
					break;
				}
				token[z++] = t;
				if(z==50){
					break;
				}
				got++;
			}
			char *ett = "usb";
			if(memcmp((char*)&token,ett,strlen(ett))==0){
				grubstatus.usb = 1;
			}
			ett = "keyboard";
			if(memcmp((char*)&token,ett,strlen(ett))==0){
				grubstatus.keyboard = 1;
			}
			if(z!=0&&t!=0x00){
				goto again;
			}
		debugf("[GRUB] Multiboot flags: %x \n" ,grub->flags);
		unsigned long fx = grub->flags;
		if(fx & 0x00000001){
			debugf("[GRUB] Memory information present!\n");
		}
		if(fx & 0x00000002){
			debugf("[GRUB] Bootdevice present!\n");
		}
		if(fx & 0x00000004){
			debugf("[GRUB] Commandline present!\n");
		}
		if(fx & 0x00000008){
			debugf("[GRUB] Modules present!\n");
		}
		if(fx & 0x00000010){
			debugf("[GRUB] Symboltable present!\n");
		}
		if(fx & 0X00000020){
			debugf("[GRUB] Elf Sectionheader present!\n");
		}
		if(fx & 0x00000040){
			debugf("[GRUB] Memorymap present!\n");
		}
		if(fx & 0x00000080){
			debugf("[GRUB] Driveinfo present!\n");
		}
		if(fx & 0x00000100){
			debugf("[GRUB] Configtable present!\n");
		}
		if(fx & 0x00000200){
			debugf("[GRUB] Bootloader name present!\n");
		}
		if(fx & 0x00000400){
			debugf("[GRUB] APMTable present!\n");
		}
		if(fx & 0x00000800){
			debugf("[GRUB] VBEInfo present!\n");
		}
		if(fx & 0x00001000){
			debugf("[GRUB] Framebuffer info present!\n");
		}
	}
	printstring("Loading utilities...\n");
	printstring("=> Programmable Interrupt Timer...\n");
	init_timer();
	printstring("=> Paging...\n");
	init_paging();
	printstring("=> Multitasking...\n");
	init_multitasking();
	printstring("=> Serial ports...\n");
	init_serial();
	printstring("=> PS2...\n");
	init_ps2();
	printstring("=> PCI...\n");
	init_pci();
	printstring("=> APCI...\n");
	init_acpi();
	printstring("=> Soundblaster...\n");
	init_soundblaster16();
	printstring("Enabeling ethernet connection\n");
	initialise_ethernet();
	debugf("Shashwat %d sss %s",1, "test2");
	printstring("\nEnd of loading system!\n");
	debugf("The system is compiled at %s %s \n",__DATE__,__TIME__);
	init_cmos();
	
	cls();
	curset(27,9);
	printf("----------------------");
	curset(27,10);
	printf("S A N D E R O S U S B ");
	curset(27,11);
	printf("----------------------");
	setForeGroundBackGround(0,7);
	for(int i = 0 ; i < 22 ; i++){
		resetTicks();
		while(getTicks()<1);
		curset(27+i,12);
		putc(' ');
	}
	setForeGroundBackGround(4,1);
	cls();
	
	if(init_graph_vga(320, 200, 1)==0) {
		printf("VGA: failed to set!\nPress any key to reboot\n");
		getch();
		poweroff();
	}
	
	if(confirm("kernel created by sander de regt, shashwat shagun, johan gericke, daniel mccarthy, jark clim and pablo narvaja") == 0) {
		poweroff();
	};

	if(!getDeviceCount()) {
		no_usefull_hardware();
	}
	
	browser();

	for(;;);
}

