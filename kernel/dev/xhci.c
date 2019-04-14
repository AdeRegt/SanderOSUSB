#include "../kernel.h"

unsigned long xhci_operational_bar = 0;

unsigned long xhci_readUSBStatusRegister(){
	return ((unsigned long *)xhci_operational_bar+0x04)[0];
}

void init_xhci(unsigned long BAR1,unsigned long BAR2,unsigned long capabilityregs){
	printf("XHCI: xhci found with BAR1 %x and BAR2 %x with capreg %x \n",BAR1,BAR2,capabilityregs);
	unsigned char* capabilities = (unsigned char*)capabilityregs;
	unsigned char capoffset = capabilities[0];
	printf("XHCI: the capabilitysize is %x \n",capabilities[0]);
	if(capoffset==0){
		printf("XHCI: the capabilityregisteroffset is 0, but should be at least 0x20. Making it 0x20...\n");
		capoffset = 0;
	}
	xhci_operational_bar = capabilityregs+capoffset;
	printf("XHCI: status register should be 1 by default and is now %x \n",xhci_readUSBStatusRegister());
	for(;;);
}
