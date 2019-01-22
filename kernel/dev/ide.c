#include "../kernel.h"

//
// ATA
//
//

extern void ideirq();

volatile int ideXirq = 0;
void irq_ide(){
	printstring("IRQ: irq for ide fired!\n");
	ideXirq = 1;
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

void waitForIDEFire(){
	while(ideXirq==0){}
}

typedef struct{
	unsigned short command;
	unsigned short control;
	unsigned char irq;
	unsigned char slave;
}IDEDevice;

#define ATAPI_SECTOR_SIZE 2048

IDEDevice ata1 = {.command= 0x1f0,.control=0x3f6,.irq=14,.slave=0};
IDEDevice ata2 = {.command= 0x1f0,.control=0x3f6,.irq=14,.slave=1};
IDEDevice ata3 = {.command= 0x170,.control=0x3f6,.irq=15,.slave=0};
IDEDevice ata4 = {.command= 0x170,.control=0x376,.irq=15,.slave=1};

unsigned char atapi_packet[12];

void atapi_eject(IDEDevice dev){
	while(inportb(dev.command+7) & 0x80){
		if(inportb(dev.command+7) & 1){
			printstring("IDE: error\n");
			return;
		}
	}
	outportb(dev.command+6, dev.slave==1?0xB0:0xA0);
	resetTicks();
	while(1){
		if(getTicks()==5){
			break;
		}
	}
	outportb(dev.command+7,0xA0);
	while(inportb(dev.command+7) & 0x80){
		if(inportb(dev.command+7) & 1){
			printstring("IDE: error\n");
			return;
		}
	}
	resetTicks();
	while(1){
		if(getTicks()==5){
			break;
		}
	}
	atapi_packet[ 0] = 0xA6;
      	atapi_packet[ 1] = 0x00;
      	atapi_packet[ 2] = 0x00;
      	atapi_packet[ 3] = 0x00;
      	atapi_packet[ 4] = 0x02;
      	atapi_packet[ 5] = 0x00;
      	atapi_packet[ 6] = 0x00;
      	atapi_packet[ 7] = 0x00;
      	atapi_packet[ 8] = 0x00;
      	atapi_packet[ 9] = 0x00;
      	atapi_packet[10] = 0x00;
      	atapi_packet[11] = 0x00;
      	ideXirq = 0;
	asm("rep   outsw"::"c"(6), "d"(dev.command), "S"(atapi_packet));
	resetTicks();
	while(1){
		if(getTicks()==5){
			break;
		}
	}
	waitForIDEFire();
}

void atapi_read_sector(IDEDevice dev,unsigned long lba,unsigned char count, unsigned long location){
	while(inportb(dev.command+7) & 0x80){
		if(inportb(dev.command+7) & 1){
			printstring("IDE: error\n");
			return;
		}
	}
	outportb(dev.command+6, dev.slave==1?0xB0:0xA0);
	resetTicks();
	while(1){
		if(getTicks()==5){
			break;
		}
	}
	outportb(dev.command+1,0);
	outportb(dev.command+4,ATAPI_SECTOR_SIZE & 0xFF);
	outportb(dev.command+5,ATAPI_SECTOR_SIZE >> 8);
	outportb(dev.command+7,0xA0);
	
	resetTicks();
	while(1){
		if(getTicks()==5){
			break;
		}
	}
	while(inportb(dev.command+7) & 0x80){
		if(inportb(dev.command+7) & 1){
			printstring("IDE: error\n");
			return;
		}
	}
	unsigned char commands[12];
	unsigned short* tau = (unsigned short*) commands;
	commands[9] = count;
	commands[0] = 0xA6;//0xA8
	commands[2] = (lba >> 0x18) & 0xff;
	commands[3] = (lba >> 0x10) & 0xff;
	commands[4] = (lba >> 0x08) & 0xff;
	commands[5] = (lba >> 0x00) & 0xff;
	ideXirq = 0;
	resetTicks();
	while(1){
		if(getTicks()==5){
			break;
		}
	}
	for(int i = 0 ; i < 6 ; i++){
		outportw(dev.command+0,tau[i]);
	}
	waitForIDEFire();
	if(inportb(dev.command+7) & 1){
		printstring("IDE: error\n");
		return;
	}
}

void init_ide_device(IDEDevice device){
	setNormalInt(device.irq,(unsigned long)ideirq);
	printstring("IDE: initialising device CMD=");
	hexdump(device.command);
	printstring(" CTRL=");
	hexdump(device.control);
	printstring(" IRQ=");
	hexdump(device.irq);
	printstring(" SLV=");
	printstring(device.slave==1?"SLAVE":"MASTER");
	printstring("\n");
	
	outportb(device.command+6,device.slave==1?0xB0:0xA0);
	outportb(device.command+2,0);
	outportb(device.command+3,0);
	outportb(device.command+4,0);
	outportb(device.command+5,0);
	outportb(device.command+7,0xEC);
	
	resetTicks();
	while(1){
		if(getTicks()==1){
			break;
		}
	}
	
	if(inportb(device.command+7)==0){
		printstring("IDE: device does not exist!\n");
		return;
	}
	
	while(1){
		if((inportb(device.command+7) & 0x80)>0){
			break;
		}else if(!(inportb(device.command+4)==0&&inportb(device.command+5)==0)){
			break;
		}
	}
	
	if(inportb(device.command+4)==0&&inportb(device.command+5)==0){
		printstring("IDE: device is ATA\n");
		for(int i = 0 ; i < 256 ; i++){
			inportw(device.command);
		}
		// ATA device detected!
	}else{
		
		// Device is NOT ATA
		// Maybe it is ATAPI?
		//if((inportb(device.command+4)==0x14)&&(inportb(device.command+5)==0xEB)){
			
		outportb(device.command+6,device.slave==1?0xB0:0xA0);
		outportb(device.command+2,0);
		outportb(device.command+3,0);
		outportb(device.command+4,0);
		outportb(device.command+5,0);
		outportb(device.command+7,0xA1);

		resetTicks();
		while(1){
			if(getTicks()==1){
				break;
			}
		}

		if(inportb(device.command+7)==0){
			printstring("IDE: device does not exist!\n");
			return;
		}
		printstring("IDE: device is ATAPI\n");
		for(int i = 0 ; i < 256 ; i++){
			inportw(device.command);
		}	
		
		//atapi_read_sector(device,1,1,0x2000);
		atapi_eject(device);
	}
}

void init_ide(unsigned short BAR){
	printstring("IDE: initialisation started!\n");
	if(BAR & 0x01){
		BAR--;
		printstring("IDE: IDE attached to IO port at base ");
		hexdump(BAR);
		printstring("\n");
	}else{
		printstring("IDE: IDE attached to memorylocation ");
		hexdump(BAR);
		printstring("\n");
		printstring("IDE: memory location not supported!!\n");
		return;
	}
	
	unsigned long atamode = inportl(BAR);
	if(atamode & 1){
		printstring("IDE: IDE controller in native mode\n");
	}else{
		printstring("IDE: IDE controller in compatebility mode\n");
		printstring("IDE: Compatebility mode not supported\n");
//		return;
	}
	
	init_ide_device(ata1);
	init_ide_device(ata2);
	init_ide_device(ata3);
	init_ide_device(ata4);
}


