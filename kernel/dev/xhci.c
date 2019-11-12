#include "../kernel.h"

extern void xhciirq();

void irq_xhci(){
	printf("XHCI: int fire\n");
}

unsigned long basebar = 0;
unsigned long usbcmd =0;
unsigned long usbsts = 0;

// offset intell xhci 0x47C
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("XHCI: entering xhci driver....\n");
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)xhciirq);
//	
//	GETTING BASIC INFO
	unsigned long deviceid = (getBARaddress(bus,slot,function,0) & 0xFFFF0000) >> 16;
	unsigned long bar = getBARaddress(bus,slot,function,0x10);
	if(deviceid==0x22B5){
		printf("[XHCI] INTELL XHCI CONTROLLER\n");
		basebar = bar+0x7C;
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
		basebar = bar + ((unsigned char*)bar)[0];
	}
	usbcmd = basebar;
	usbsts = basebar+0x04;
	
	unsigned long xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	if((xhci_usbcmd & 1)==1){
		printf("[XHCI] controller already running! (%x)\n",xhci_usbcmd);
		// ask controller to stop
		resetTicks();
		((unsigned long*)usbcmd)[0] &= ~1;
		while(getTicks()<5);
		xhci_usbcmd = ((unsigned long*)usbcmd)[0];
		xhci_usbsts = ((unsigned long*)usbsts)[0];
		if((xhci_usbsts & 1)==1&&(xhci_usbcmd & 1)==0){
			printf("[XHCI] stopping of hostcontroller succeed\n");
		}else{
			printf("[XHCI] failed to halt hostcontroller %x %x \n",xhci_usbcmd,xhci_usbsts);
		}
	}
	
	//
	// lets reset!
	printf("[XHCI] Resetting XHCI \n");
	resetTicks();
	((unsigned long*)usbcmd)[0] |= 2;
	while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] Reset XHCI finished with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
	if(xhci_usbsts & 0b10000){
		printf("[XHCI] Portchange detected!\n");
	}
	printf("[XHCI] Probing ports....\n");
	for(int i = 0 ; i < 10 ; i++){
		unsigned long map = basebar + 0x400 + (i*0x10);
		unsigned long val = ((unsigned long*)map)[0];
		if(val&3){
			printf("[XHCI] Port %x seems to have a connection\n",i);
		}
	}
	printf("[XHCI] All finished!\n");
	for(;;);
}
