#include "../kernel.h"
#define FRAMELIST_MAX_SIZE 1024
	
unsigned long USBCMD;
unsigned long USBSTS;
unsigned long USBINT;
unsigned long FRNUM;
unsigned long FLBSAD;
unsigned long SOF;
unsigned long PORT01;
unsigned long PORT02;

unsigned long framelist[FRAMELIST_MAX_SIZE] __attribute__ ((aligned(0x1000)));
unsigned char framelistpointer = 0;

void uhci_ring_doorbell(){
	unsigned long tx = inportw(FRNUM);
	printf("[UHCI] Set RUN bit [ %x ]\n",tx);
	outportw(USBCMD,0x00);
	sleep(500);
	outportw(FRNUM,0);
	sleep(500);
	outportw(USBCMD,0x01);
	sleep(1000);
	//sleep(50);
	//outportw(USBCMD,0x00);
}

int uhci_init_port(unsigned long port,int i){
	unsigned short portvalue = inportw(port);
	printf("[UHCI] Port %x : portstatus=%x \n",i,portvalue);
	unsigned char current_connect_status 	= (portvalue & 0b0000000000000001) >> 0;
	unsigned char connect_status_change 	= (portvalue & 0b0000000000000010) >> 1;
	unsigned char port_enabled 		= (portvalue & 0b0000000000000100) >> 2;
	unsigned char port_enabled_change 	= (portvalue & 0b0000000000001000) >> 3;
	unsigned char port_status 		= (portvalue & 0b0000000000110000) >> 4;
	unsigned char resume_detect 		= (portvalue & 0b0000000001000000) >> 6;
	unsigned char low_speed 		= (portvalue & 0b0000000100000000) >> 8;
	unsigned char port_reset 		= (portvalue & 0b0000001000000000) >> 9;
	unsigned char suspend 			= (portvalue & 0b0001000000000000) >> 12;
	if(current_connect_status){
		printf("[UHCI] Port %x : Status is connected\n",i);
	}
	if(connect_status_change){
		printf("[UHCI] Port %x : Connect status is changed\n",i);
	}
	if(port_enabled){
		printf("[UHCI] Port %x : Port is enabled\n",i);
	}
	if(port_enabled_change){
		printf("[UHCI] Port %x : Change in port enabled\n",i);
	}
	if(port_status){
		printf("[UHCI] Port %x : Port status is %x \n",i,port_status);
	}
	if(resume_detect){
		printf("[UHCI] Port %x : Resume detected\n",i);
	}
	if(low_speed){
		printf("[UHCI] Port %x : Low speed\n",i);
	}
	if(port_reset){
		printf("[UHCI] Port %x : Port in reset\n",i);
	}
	if(suspend){
		printf("[UHCI] Port %x : Suspend\n",i);
	}
	
	if((connect_status_change||port_enabled_change)==0){
		return 0;
	}
	
	printf("[UHCI] Port %x : Resetting port...\n",i);
	portvalue |= 0b0000001000000000;
	portvalue = (portvalue & 0x324E) + (1<<7);
	outportw(port,portvalue);
	sleep(500);
	outportw(port,(portvalue & 0b1111110111111111));
	return 1;
}

extern void uhciirq();

void irq_uhci(){
	printf("[UHCI] Int fired!\n");
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

void uhci_init(int bus,int slot,int function){
	printf("[UHCI] UHCI initialised\n");
	
	unsigned long base = getBARaddress(bus,slot,function,0x20) & 0b00000000000000001111111111100000;
	printf("[UHCI] Base address at %x \n",base);
	
	unsigned long version = getBARaddress(bus,slot,function,0x60);
	printf("[UHCI] Version is %x \n",version);
	if(version!=0x10){
		printf("[UHCI] Version not supported\n");
		return;
	}
	
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	printf("[UHCI] Installing interrupts at %x \n",usbint);
	setNormalInt(usbint,(unsigned long)uhciirq);
	
	unsigned short deviceid = (getBARaddress(bus,slot,function,0) & 0xFFFF0000) >> 16;
	printf("[UHCI] Deviceid=%x \n",deviceid);
	if(deviceid==0x7020){
		printf("[UHCI] UHCI-device comes from BOCHS. Halting\n");
		return;
	}
	
	USBCMD = base+0x00;
	USBSTS = base+0x02;
	USBINT = base+0x04;
	FRNUM  = base+0x06;
	FLBSAD = base+0x08;
	SOF    = base+0x0C;
	PORT01 = base+0x10;
	PORT02 = base+0x12;
	
	printf("[UHCI] Global reset\n");
	outportw(USBCMD,4);
	sleep(100);
	outportw(USBCMD,0);
	printf("[UHCI] Global reset finished!\n");
	outportw(USBSTS,0);
	outportw(USBINT,0);
	outportw(FRNUM,0);
	for(int i = 0 ; i < FRAMELIST_MAX_SIZE ; i++){
		framelist[i] = 0x3;
	}
	outportl(FLBSAD,(unsigned long)&framelist);
	outportb(SOF,0x40);
	uhci_ring_doorbell();
	printf("[UHCI] Detecting ports\n");
	uhci_init_port(PORT01,1);
	uhci_init_port(PORT02,2);
	printf("[UHCI] That is it for now\n");
}
