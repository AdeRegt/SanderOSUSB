#include "../kernel.h"

unsigned long operationalregistersoffset;

unsigned long get_xhci_USBCMD(){
	return ((unsigned long*)operationalregistersoffset)[0];
}

unsigned long get_xhci_USBSTS(){
	unsigned long ta = operationalregistersoffset+0x04;
	return ((unsigned long*)ta)[0];
}

unsigned long get_xhci_PAGESIZE(){
	unsigned long ta = operationalregistersoffset+0x08;
	return ((unsigned long*)ta)[0];
}

unsigned long get_xhci_DNCTRL(){
	unsigned long ta = operationalregistersoffset+0x14;
	return ((unsigned long*)ta)[0];
}

unsigned long get_xhci_CRCR(){
	unsigned long ta = operationalregistersoffset+0x18;
	return ((unsigned long*)ta)[0];
}

unsigned long get_xhci_DCBAAP(){
	unsigned long ta = operationalregistersoffset+0x30;
	return ((unsigned long*)ta)[0];
}

unsigned long get_xhci_CONFIG(){
	unsigned long ta = operationalregistersoffset+0x38;
	return ((unsigned long*)ta)[0];
}

unsigned long get_xhci_PORTSC(int i){
	unsigned long ta = operationalregistersoffset+0x400+(i*0x0C);
	return ((unsigned long*)ta)[0];
}

extern void xhciirq();

void irq_xhci(){
	printf("XHCI: int fire\n");
	if(get_xhci_USBSTS() & 0b00000000000000000000000000000100){printf("XHCI: host system error\n");}
	if(get_xhci_USBSTS() & 0b00000000000000000000000000001000){printf("XHCI: event interrupt\n");}
	if(get_xhci_USBSTS() & 0b00000000000000000000000000010000){printf("XHCI: portchange\n");}
	// EOI
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("XHCI: entering xhci driver....\n");
	unsigned long base1 = getBARaddress(bus,slot,function,0x10);
	unsigned long base2 = getBARaddress(bus,slot,function,0x14);
	printf("XHCI: base addr (64bit?) 0x%x%x \n",base2,base1);
	unsigned long base = base1;
	unsigned long interruptline = getBARaddress(bus,slot,function,0x3C) & 0xFF;
	printf("XHCI: assigned interrupt line is %x \n",interruptline);
	setNormalInt(interruptline,(unsigned long)xhciirq);
	unsigned long capabilityoffset = (getBARaddress(bus,slot,function,0x34) & 0xFF)+base;
	printf("XHCI: capabilityregisterslocation %x \n",capabilityoffset);
	unsigned short capabilitysize = ((unsigned short*)capabilityoffset)[0];
	printf("XHCI: capabilitysize %x \n",capabilitysize);
	operationalregistersoffset = capabilityoffset+capabilitysize;
	printf("XHCI: offset of operational registers %x \n",operationalregistersoffset);
	((unsigned long*)operationalregistersoffset)[0] = 0b00000000000000000000000000000010;
	checkagain:
	if(get_xhci_USBCMD() & 0b00000000000000000000000000000010){
		goto checkagain;
	}
	printf("XHCI: starting value USBCMD %x \n",get_xhci_USBCMD());
	printf("XHCI: starting value USBSTS %x \n",get_xhci_USBSTS());
	for(int i = 0 ; i < 10 ;i++){
		unsigned long port = get_xhci_PORTSC(i);
		if(port & 3){
			printf("XHCI: port %x has a connection: %x \n",i,port);
		}
	}
	for(;;);
}
