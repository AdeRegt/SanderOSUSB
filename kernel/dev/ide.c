#include "../kernel.h"

//
// ATA
//
//

extern void ideirq();
extern void iso_9660_dir();
extern void iso_9660_read();

volatile int ideXirq = 0;
void irq_ide(){
	//printstring("IRQ: irq for ide fired!\n");
	ideXirq = 1;
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

void resetIDEFire(){
	ideXirq=0;
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

char getIDEError(IDEDevice cdromdevice){
	unsigned char msg = inportb(cdromdevice.command+7);
	if((msg >> 0 ) & 1){
		if(msg & 0x80){
			printf("IDE: Bad sector\n");
		}else if(msg & 0x40){
			printf("IDE: Uncorrectable data\n");
		}else if(msg & 0x20){
			printf("IDE: No media\n");
		}else if(msg & 0x10){
			printf("IDE: ID mark not found\n");
		}else if(msg & 0x08){
			printf("IDE: No media\n");
		}else if(msg & 0x04){
			printf("IDE: Command aborted\n");
		}else if(msg & 0x02){
			printf("IDE: Track 0 not found\n");
		}else if(msg & 0x01){
			printf("IDE: No address mark\n");
		}
		return 1;
	}
	return 0;
}


char read_cmd[12] = { 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void atapi_eject(IDEDevice cdromdevice){
	getIDEError(cdromdevice);
    	while((inportb (cdromdevice.command+7)) & 0x80){}
	outportb(cdromdevice.command+6,cdromdevice.slave==1?0xB0:0xA0);
    	outportb(cdromdevice.command+1,0x00);
    	outportb(cdromdevice.command+7,0xA0);
    	
    	getIDEError(cdromdevice);
    	while((inportb (cdromdevice.command+7)) & 0x80){}
    	
    	read_cmd[ 0] = 0x1B;
    	read_cmd[ 1] = 0x00;
    	read_cmd[ 2] = 0x00;
    	read_cmd[ 3] = 0x00;
    	read_cmd[ 4] = 0x02;
    	read_cmd[ 5] = 0x00;
    	read_cmd[ 6] = 0x00;
    	read_cmd[ 7] = 0x00;
    	read_cmd[ 8] = 0x00;
    	read_cmd[ 9] = 0x00;
    	read_cmd[10] = 0x00;
    	read_cmd[11] = 0x00;
    	 
    	resetIDEFire();
    	getIDEError(cdromdevice);
    	unsigned short *mdx = (unsigned short*)&read_cmd;
    	for(int f = 0 ; f < 6 ; f++){
        	outportw(cdromdevice.command+0,mdx[f]);
    	}
    	getIDEError(cdromdevice);
    	waitForIDEFire();
    	while((inportb (cdromdevice.command+7)) & 0x80){}
    	getIDEError(cdromdevice);
    	while((inportb (cdromdevice.command+7)) & 0x80){}
}

void atapi_read_sector(IDEDevice cdromdevice,unsigned long lba,unsigned char count, unsigned short *location){
	//printf("ATAPI: reading LBA %x and copy to %x \n",lba,location);
	getIDEError(cdromdevice);
    	while((inportb (cdromdevice.command+7)) & 0x80){}
	outportb(cdromdevice.command+6,cdromdevice.slave==1?0xB0:0xA0);
    	outportb(cdromdevice.command+1,0x00);
    	outportb(cdromdevice.command+4,ATAPI_SECTOR_SIZE & 0xff);
    	outportb(cdromdevice.command+5,ATAPI_SECTOR_SIZE >> 8);
    	outportb(cdromdevice.command+7,0xA0);
    	
    	getIDEError(cdromdevice);
    	while((inportb (cdromdevice.command+7)) & 0x80){}
    	
    	read_cmd[9] = count;
    	read_cmd[2] = (lba >> 0x18) & 0xFF;   /* most sig. byte of LBA */
    	read_cmd[3] = (lba >> 0x10) & 0xFF;
    	read_cmd[4] = (lba >> 0x08) & 0xFF;
    	read_cmd[5] = (lba >> 0x00) & 0xFF;
    	read_cmd[0] = 0xA8;
    	 
    	resetIDEFire();
    	getIDEError(cdromdevice);
    	unsigned short *mdx = (unsigned short*)&read_cmd;
    	for(int f = 0 ; f < 6 ; f++){
        	outportw(cdromdevice.command+0,mdx[f]);
    	}
    	getIDEError(cdromdevice);
    	waitForIDEFire();
    	unsigned short size = (((int) inportb(cdromdevice.command+5)) << 8) | (int) (inportb(cdromdevice.command+4));
//    	if(size!=(count*ATAPI_SECTOR_SIZE)){
//    		printf("SIZE IS OTHER AS COUNT: EXP %x FND %x ",size,(count*ATAPI_SECTOR_SIZE));
//    	}
    	while((inportb (cdromdevice.command+7)) & 0x80){}
    	int mp = 0;
    	for (unsigned short i = 0; i < (size / 2); i++) {
    		if(getIDEError(cdromdevice)==1){return;}
    		location[mp++] = inportw(cdromdevice.command+0);
//        	unsigned short d = inportw(cdromdevice.command+0);
//        	unsigned char A = d;
//        	unsigned char B = (unsigned char)(d >> 8);
//        	((unsigned char*)location)[mp++] = A;
//        	((unsigned char*)location)[mp++] = B;
        }
    	while((inportb (cdromdevice.command+7)) & 0x80){}
}

void atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location){
	IDEDevice ide;
	ide.command 	= dev->arg1;
	ide.control 	= dev->arg2;
	ide.irq 	= dev->arg3;
	ide.slave 	= dev->arg4;
	atapi_read_sector(ide,lba,count,location);
}

char issata = 0;

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
	
	if(inportb(0x1F4)==0x3C || inportb(0x1F5)==0xC3){
		printf("IDE: Device is SATA\n");
		issata=1;
		return;
	}
	
	if(inportb(device.command+4)==0&&inportb(device.command+5)==0){
		printstring("IDE: device is ATA\n");
		for(int i = 0 ; i < 256 ; i++){
			inportw(device.command);
		}
		// ATA device detected!
	}else{
		for(int i = 0 ; i < 256 ; i++){
			inportw(device.command);
		}
		
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
		if(getIDEError(device)==0){
			printstring("IDE: device is ATAPI\n");
			for(int i = 0 ; i < 256 ; i++){
				inportw(device.command);
			}
			unsigned char *buffer = (unsigned char*) 0x2000;
			atapi_read_sector(device,0,1, (unsigned short *)buffer);
			if(buffer[510]==0x55&&buffer[511]==0xAA){
				printf("ATAPI: cdrom is bootable!\n");
			}else{
				printf("ATAPI: cdrom is not bootable!\n");
			}
			int choice = -1;
			for(int i = 0 ; i < 10 ; i++){
				atapi_read_sector(device,0x10+i,1, (unsigned short *)buffer);
				if(buffer[1]=='C'&&buffer[2]=='D'&&buffer[3]=='0'&&buffer[4]=='0'&&buffer[5]=='1'){
					choice = i;
					break;
				}
			}
			if(choice==-1){
				printf("ATAPI: unknown filesystem\n");
			}else{
				printf("ATAPI: known filesystem ISO 9660\n");
				
				Device *regdev = getNextFreeDevice();
				
				regdev->readRawSector 	= (unsigned long)&atapi_read_raw;
//				regdev->writeRawSector 	= (unsigned long)&atapi_write_raw;
//				regdev->reinitialise 	= (unsigned long)&atapi_reset_raw;
//				regdev->eject 		= (unsigned long)&atapi_eject_raw;
				
				regdev->dir		= (unsigned long)&iso_9660_dir;
				regdev->readFile	= (unsigned long)&iso_9660_read;
				
				// .command= 0x1f0,.control=0x3f6,.irq=14,.slave=0
				regdev->arg1 = device.command;
				regdev->arg2 = device.control;
				regdev->arg3 = device.irq;
				regdev->arg4 = device.slave;
				regdev->arg5 = ATAPI_SECTOR_SIZE;
			}
		}
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
	if(issata==0){
		init_ide_device(ata2);
		init_ide_device(ata3);
		init_ide_device(ata4);
	}
}
