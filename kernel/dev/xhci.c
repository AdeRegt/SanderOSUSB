#include "../kernel.h"

typedef struct{
	unsigned long bar1;
	unsigned long bar2;
	unsigned long bar3;
	unsigned long bar4;
}TRB;

extern void xhciirq();

unsigned long basebar = 0;
unsigned long usbcmd = 0;
unsigned long usbsts = 0;
unsigned long config = 0;
unsigned long bcbaap = 0;
unsigned long crcr   = 0;
unsigned long doorbel= 0;
unsigned long dnctrl = 0;
unsigned long erstsz = 0;
unsigned long erstba = 0;
unsigned long rtsoff = 0;
TRB* ring = ((TRB*)0x1500);
TRB evts[10];

void irq_xhci(){
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	if(xhci_usbsts&4){
		printf("[XHCI] Host system error interrupt\n");
	}
	if(xhci_usbsts&8){
		printf("[XHCI] Event interrupt\n");
	}
	if(xhci_usbsts&0x10){
		printf("[XHCI] Port interrupt\n");
	}
	unsigned long iman_addr = rtsoff + 0x020;
	((unsigned long*)iman_addr)[0] = 1;
	((unsigned long*)iman_addr)[0] &= ~1;
	((unsigned long*)iman_addr)[0] |= 2;
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

// offset intell xhci 0x47C
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("[XHCI] entering xhci driver....\n");
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)xhciirq);
//	
//	GETTING BASIC INFO
	unsigned long deviceid = (getBARaddress(bus,slot,function,0) & 0xFFFF0000) >> 16;
	unsigned long bar = getBARaddress(bus,slot,function,0x10);
	unsigned long capdb = bar+0x14;
	unsigned long hciparamadr = bar+0x04;
	unsigned long hciparam1 = ((unsigned long*)hciparamadr)[0];
	unsigned long portcount = hciparam1>>24;
	unsigned long rtsoffa = bar+0x18;
	rtsoff = bar+(((unsigned long*)rtsoffa)[0]);
	printf("[XHCI] Runtime offset %x \n",rtsoff);
	printf("[XHCI] portcount %x \n",portcount);
	doorbel = bar+((unsigned long*)capdb)[0];
	printf("[XHCI] doorbell offset %x \n",doorbel);
//
// Calculating base address
	if(deviceid==0x22B5){
		printf("[XHCI] INTELL XHCI CONTROLLER\n");
		basebar = bar+0x7C;
		rtsoff += 0xC;
	}else if(deviceid==0xD){
		printf("[XHCI] QEMU XHCI CONTROLLER\n");
		unsigned long premature = bar + (getBARaddress(bus,slot,function,0x34) & 0x000000FF);
		basebar = premature+((unsigned char*)premature)[0];
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
		basebar = bar + ((unsigned char*)bar)[0];
	}
//
// Calculating other addresses
	usbcmd = basebar;
	usbsts = basebar+0x04;
	config = basebar+0x38;
	bcbaap = basebar+0x30;
	crcr   = basebar+0x18;
	dnctrl = basebar+0x14;
	
	printf("[XHCI] default value CONFIG %x \n",((unsigned long*)config)[0]);
	printf("[XHCI] default value BCBAAP %x \n",((unsigned long*)bcbaap)[0]);
	printf("[XHCI] default value CRCR   %x \n",((unsigned long*)crcr)[0]);
	
	unsigned long xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
//
// Stopping controller when running
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
// Lets reset!
	printf("[XHCI] Resetting XHCI \n");
	resetTicks();
	((unsigned long*)usbcmd)[0] |= 2;
	while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] Reset XHCI finished with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
//
// Setup default parameters
	// TELL XHCI TO USE INTERRUPTS
	((unsigned long*)usbcmd)[0] |= 4;
	// TELL XHCI TO REPORT EVERYTHING
	((unsigned long*)dnctrl)[0] |= 0b1111111111111111;
	
	// setting up interrupter management register
	// setting first interrupt enabled.
	unsigned long iman_addr = rtsoff + 0x020;
//	((unsigned long*)iman_addr)[0] |= 0b10; // Interrupt Enable (IE) – RW
	
	// setting up "Event Ring Segment Table Size Register (ERSTSZ)"
	unsigned long erstsz_addr = rtsoff + 0x028;
	((unsigned long*)erstsz_addr)[0] |= 1; // keep only 1 open
	
	// setting up "Event Ring Segment Table"
	unsigned long rsb1 = 0x1000;
	unsigned long rsb2 = 0x1000+8;
	((unsigned long*)rsb1)[0] |= 0x41400;
	((unsigned long*)rsb2)[0] |= 16;
	
	// setting up "Event Ring Segment Table Base Address Register (ERSTBA)"
	unsigned long erstba_addr = rtsoff + 0x030;
	((unsigned long*)erstba_addr)[0] |= 0x1000;//0x40000; // table at 0x1000 for now
	
	// setting up "Event Ring Dequeue Pointer Register (ERDP)"
	unsigned long erdp_addr = rtsoff + 0x038;
	((unsigned long*)erdp_addr)[0] = 0x41400;
	
	// setting up "Command Ring Control Register (CRCR)"
	unsigned long bse = 0x1500;
	unsigned long bse1 = bse<<6;
	((unsigned long*)crcr)[0] |= bse1;
	
	// DCBAAP
	unsigned long bcbaapt[10];
	unsigned long btc = (unsigned long)&bcbaapt;
	((unsigned long*)bcbaap)[0] = (unsigned long)btc;
	
	resetTicks();
	while(getTicks()<5);
//
// Start system
	((unsigned long*)usbcmd)[0] |= 1;
	
	resetTicks();
	while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] System up and running with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
	
	if(xhci_usbsts & 0b10000){
		printf("[XHCI] Portchange detected!\n");
	}
	printf("[XHCI] Probing ports....\n");
	//
	// First, see which ports are available
	// USB3.0 will say automatically he is there.
	// USB2.0 should be activated manually
	//
	// After this we should initialise it by getting its port number from the ring
	unsigned int att = 0;
	for(int i = 0 ; i < 5 ; i++){
		unsigned long map = basebar + 0x400 + (i*0x10);
		unsigned long val = ((unsigned long*)map)[0];
		if(val&3){ // USB 3.0 does everything themselves
			printf("[XHCI] Port %x has a USB3.0 connection (%x)\n",i,val);
			att++;
		}else{ // USB 2.0 however doesnt...
			((unsigned long*)map)[0] = 0b1000010000; // activate power and reset port
			resetTicks();
			while(getTicks()<5);
			val = ((unsigned long*)map)[0];
			if(val&3){
				printf("[XHCI] Port %x has a USB2.0 connection (%x)\n",i,val);
				att++;
			}
		}
		
		
		if(val==0){
			break;
		}
		
		if(val&3){
			printf("[XHCI] Port %x is initialising....\n",i);
			//
			// setting up TRB
			
			TRB* trb2 = ((TRB*)0x54000);
			trb2->bar1 = 0;
			trb2->bar2 = 0;
			trb2->bar3 = 0;
			trb2->bar4 = 0b00000000000000000010010000000000;
			
			//
			// STOP CODON
			TRB* trb = ((TRB*)0x54010);
			trb->bar1 = 0;
			trb->bar2 = 0;
			trb->bar3 = 0;
			trb->bar4 = 1;
			
			// doorbel
			unsigned long tingdongaddr = doorbel;
			((unsigned long*)tingdongaddr)[0] = 0;
			
			// wait
			resetTicks();
			while(getTicks()<10);
			
			// result
			TRB* trbres = ((TRB*)0x1050);
			printf("[XHCI] %x %x %x %x \n",trbres->bar1,trbres->bar2,trbres->bar3,trbres->bar4);
			trbres = ((TRB*)0x41400);
			printf("[XHCI] %x %x %x %x \n",trbres->bar1,trbres->bar2,trbres->bar3,trbres->bar4);
			trbres = ((TRB*)0x1050+0x10);
			printf("[XHCI] %x %x %x %x \n",trbres->bar1,trbres->bar2,trbres->bar3,trbres->bar4);
			trbres = ((TRB*)0x41400+0x10);
			printf("[XHCI] %x %x %x %x \n",trbres->bar1,trbres->bar2,trbres->bar3,trbres->bar4);
		}
	}
	
	if(((unsigned long*)crcr)[0]==0x8){
		printf("[XHCI] circulair command ring is running\n");
	}
	printf("[XHCI] All finished!\n");
	if(att>0){
		for(;;);
	}
}
