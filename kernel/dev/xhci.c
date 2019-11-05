#include "../kernel.h"

unsigned long xhci_base = 0;
unsigned long xhci_usbcmd = 0;
unsigned long xhci_usbsts = 0;
unsigned long xhci_ports_offset = 0;
unsigned long xhci_config = 0;
unsigned long xhci_pagesize = 0;
unsigned long xhci_dnctrl = 0;

extern void xhciirq();

void dumpPortstati(){
	for(int i = 0 ; i < 10 ; i++){
		unsigned long tar = ((unsigned char*)xhci_ports_offset+(i*0x10))[0];
		if(tar & 0b11){ // attached or connected?
			printf("XHCI: PORT%x: %x attached \n",i,tar);
		}else if(tar == 0x00){ // ignore
		
		}else if(tar != 0xA0){ // unknown status?
			printf("XHCI: PORT%x: %x \n",i,tar);
		}else { // force setpower on port
			((unsigned char*)xhci_ports_offset+(i*0x10))[0] = 0b1000000000;
		}
	}
}

void irq_xhci(){
	printf("XHCI: int fire\n");
	printf("XHCI: USBCMD %x USBSTS %x \n",((unsigned long*)xhci_usbcmd)[0],((unsigned long*)xhci_usbsts)[0]);
	dumpPortstati();
}

// offset intell xhci 0x47C
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("XHCI: entering xhci driver....\n");
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)xhciirq);
//	
//	GETTING BASIC INFO
	unsigned long capabilityregister = getBARaddress(bus,slot,function,0x34) & 0x000000FF;
	unsigned long bar = getBARaddress(bus,slot,function,0x10);
	printf("XHCI: capreg %x bar %x \n",capabilityregister,bar);
	unsigned long capabilityregistersizeloc = bar+capabilityregister;
	unsigned long capabilityregistersize = ((unsigned char*)capabilityregistersizeloc)[0];
	printf("XHCI: caplength %x \n",capabilityregistersize);
	xhci_base = capabilityregistersize+capabilityregistersizeloc;
	printf("XHCI: registers at %x \n",xhci_base);
	xhci_usbcmd = xhci_base;
	xhci_usbsts = xhci_base+0x04;
	xhci_ports_offset = xhci_base+0x400;
	printf("XHCI: USBCMD %x USBSTS %x \n",((unsigned long*)xhci_usbcmd)[0],((unsigned long*)xhci_usbsts)[0]);

	//
	// resetting
	((unsigned long*)xhci_usbcmd)[0] |= 2;
	resetTicks();
	while(((unsigned long*)xhci_usbcmd)[0]){
		if(getTicks()==10){
			printf("XHCI: TIMEOUT\n");
			break;
		}
	}
	xhci_config = xhci_base + 0x38;
	xhci_pagesize = xhci_base + 0x08;
	xhci_dnctrl = xhci_base + 0x14;
	
	// set 5 ports
	((unsigned long*)xhci_config)[0] = 5;
	// report everything
	((unsigned long*)xhci_dnctrl)[0] |= 0b00000000000000001111111111111111;
	// run
	((unsigned long*)xhci_usbcmd)[0] |= 1;
	
	printf("\n[READY]\n\n");
	dumpPortstati();
	for(;;);
}
