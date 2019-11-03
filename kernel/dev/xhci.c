#include "../kernel.h"

unsigned long xhci_base = 0;
unsigned long xhci_usbcmd = 0;
unsigned long xhci_usbsts = 0;
unsigned long xhci_ports_offset = 0;

extern void xhciirq();

void dumpPortstati(){
	for(int i = 0 ; i < 10 ; i++){
		unsigned long tar = ((unsigned char*)xhci_ports_offset+(i*0x10))[0];
		if(tar & 1){
			printf("XHCI: PORT%x: %x \n",i,tar);
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
//	GETTING RIGHT OFFSETS
	// is this offset already right?
	if(((unsigned long*)xhci_usbsts)[0]==1){
		// yes, do not change
		printf("XHCI: default settings are OK\n");
	}else{
		bar = getBARaddress(bus,slot,function,0x10);
		printf("XHCI: bar %x \n",bar);
		capabilityregistersizeloc = bar;
		capabilityregistersize = ((unsigned char*)capabilityregistersizeloc)[0];
		printf("XHCI: caplength %x \n",capabilityregistersize);
		xhci_base = capabilityregistersize+capabilityregistersizeloc;
		printf("XHCI: registers at %x \n",xhci_base);
		xhci_usbcmd = xhci_base;
		xhci_usbsts = xhci_base+0x04;
		xhci_ports_offset = xhci_base+0x40C;
		printf("XHCI: USBCMD %x USBSTS %x \n",((unsigned long*)xhci_usbcmd)[0],((unsigned long*)xhci_usbsts)[0]);
		if(((unsigned long*)xhci_usbsts)[0]==1){
			// yes, do not change
			printf("XHCI: discoverymethod 2 is OK\n");
		}else{
			bar = getBARaddress(bus,slot,function,0x10);
			printf("XHCI: bar %x \n",bar);
			capabilityregistersizeloc = 0x80;
			printf("XHCI: predefined offset %x \n",capabilityregistersizeloc);
			xhci_base = bar+capabilityregistersizeloc;
			printf("XHCI: registers at %x \n",xhci_base);
			xhci_usbcmd = xhci_base;
			xhci_usbsts = xhci_base+0x04;
			xhci_ports_offset = xhci_base+0x40C;
			printf("XHCI: USBCMD %x USBSTS %x \n",((unsigned long*)xhci_usbcmd)[0],((unsigned long*)xhci_usbsts)[0]);
			if(((unsigned long*)xhci_usbsts)[0]==1){
				// yes, do not change
				printf("XHCI: discoverymethod 3 is OK\n");
			}else{
				printf("XHCI: cannot resolve settings\n");
				return;
			}
		}
	}
	
	//
	//
	// getting all addresses
	((unsigned long*)xhci_usbcmd)[0] = 2;
	resetTicks();
	while(((unsigned long*)xhci_usbcmd)[0]){
		if(getTicks()==10){
			printf("XHCI: TIMEOUT\n");
			for(;;);
		}
	}
	
	printf("\n[READY]\n\n");
	dumpPortstati();
	for(;;);
}
