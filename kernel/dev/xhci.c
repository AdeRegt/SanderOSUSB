#include "../kernel.h"

struct XHCI_CapibilityRegisters{
	unsigned char CAPLENGTH;
	unsigned char rsvd;
	unsigned short HCIVERSION;
	unsigned long HCSPARAMS1;
	unsigned long HCSPARAMS2;
	unsigned long HCSPARAMS3;
	unsigned long HCCPARAMS1;
	unsigned long DBOFF;
	unsigned long RTSOFF;
	unsigned long HCCPARAMS2;
}__attribute__((packed));

struct XHCI_OperationalRegisters{
	unsigned long USBCMD;
	unsigned long USBSTS;
	unsigned long PAGESIZE;
	unsigned long reserverd1[2];
	unsigned long DNCTRL;
	unsigned long CRCR1;
	unsigned long CRCR2;
	unsigned long reserverd2[4];
	unsigned long DCBAAP1;
	unsigned long DCBAAP2;
	unsigned long CONFIG;
}__attribute__((packed));

struct XHCI_CapibilityRegisters *capability;
volatile struct XHCI_OperationalRegisters *operational;

void irq_xhci(){}

void xhci_dump_operational(volatile struct XHCI_OperationalRegisters *operational){
	printf("=========OPERATIONAL============\n");
	printf("[XHCI] USBCMD    = %x \n",operational->USBCMD);
	printf("[XHCI] USBSTS    = %x \n",operational->USBSTS);
	printf("[XHCI] PAGESIZE  = %x \n",operational->PAGESIZE);
	printf("[XHCI] DNCTRL    = %x \n",operational->DNCTRL);
	printf("[XHCI] CRCR      = %x %x \n",operational->CRCR1,operational->CRCR2);
	printf("[XHCI] DCBAAP    = %x %x \n",operational->DCBAAP1,operational->DCBAAP2);
	printf("[XHCI] CONFIG    = %x \n",operational->CONFIG);
}

void xhci_dump_capibility(struct XHCI_CapibilityRegisters *capability){
	printf("=========CAPIBILITY=============\n");
	printf("[XHCI] CAPLENGTH   = %x \n",capability->CAPLENGTH);
	printf("[XHCI] HCIVERSION  = %x \n",capability->HCIVERSION);
	printf("[XHCI] HCSPARAMS1  = %x \n",capability->HCSPARAMS1);
	printf("[XHCI] HCSPARAMS2  = %x \n",capability->HCSPARAMS2);
	printf("[XHCI] HCSPARAMS3  = %x \n",capability->HCSPARAMS3);
	printf("[XHCI] HCCPARAMS1  = %x \n",capability->HCCPARAMS1);
	printf("[XHCI] DBOFF       = %x \n",capability->DBOFF);
	printf("[XHCI] RTSOFF      = %x \n",capability->RTSOFF);
	printf("[XHCI] HCCPARAMS2  = %x \n",capability->HCCPARAMS2);
}

void xhci_wait_for_hostcontroller(){
	while(operational->USBSTS & 0b100000000000);
}

void xhci_reset(){
	//
	// stop if we are not stopped yet
	if(operational->USBCMD & 1){
		printf("[XHCI] Service aleady running\n");
		operational->USBCMD &= ~1;
		while(!(operational->USBSTS&1));
		printf("[XHCI] Service stopped\n");
	}

	//
	// reset!
	xhci_wait_for_hostcontroller();
	operational->USBCMD |= 0b10;
	while(operational->USBCMD&0b10); 
	printf("[XHCI] Reset finished\n");
}

void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	unsigned long bar1 = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFF0;
	unsigned long bar2 = getBARaddress(bus,slot,function,0x14);

	//
	// display version info
	printf("[XHCI] XHCI driver started at address %x %x \n",bar1,bar2);
	if(bar2){
		printf("[XHCI] XHCI driver is in 64bit mode while we are 32bit\n");
		return;
	}
	unsigned char sbrn = getBARaddress(bus,slot,function,0x60) & 0xFF;
	printf("[XHCI] SBRN=%x \n",sbrn);

	//
	// what is our default state?
	capability = (struct XHCI_CapibilityRegisters*) bar1;
	xhci_dump_capibility(capability);
	operational = (volatile struct XHCI_OperationalRegisters *) (bar1 + capability->CAPLENGTH);
	xhci_dump_operational(operational);
	printf("[XHCI] We have %x ports available to us!\n",(capability->HCCPARAMS1 >> 24) & 0xFF );

	xhci_reset();
	
	printf("[XHCI] Hang....\n");
	for(;;);
}
