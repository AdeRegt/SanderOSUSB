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
TRB ring[10];
TRB evts[10];

void irq_xhci(){
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] fire!\n");
	if(xhci_usbsts&4){
		printf("[XHCI] Host system error interrupt\n");
	}
	if(xhci_usbsts&8){
		printf("[XHCI] Event interrupt\n");
	}
	if(xhci_usbsts&0x10){
		printf("[XHCI] Port interrupt\n");
	}
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
	unsigned long rtsoff = bar+(((unsigned long*)rtsoffa)[0]);
	printf("[XHCI] Runtime offset %x \n",rtsoff);
	printf("[XHCI] portcount %x \n",portcount);
	doorbel = bar+((unsigned long*)capdb)[0];
	printf("[XHCI] doorbell offset %x \n",doorbel);
	if(deviceid==0x22B5){
		printf("[XHCI] INTELL XHCI CONTROLLER\n");
		basebar = bar+0x7C;
	}else if(deviceid==0xD){
		printf("[XHCI] QEMU XHCI CONTROLLER\n");
		unsigned long premature = bar + (getBARaddress(bus,slot,function,0x34) & 0x000000FF);
		basebar = premature+((unsigned char*)premature)[0];
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
		basebar = bar + ((unsigned char*)bar)[0];
	}
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
	// TELL XHCI TO USE INTERRUPTS
	((unsigned long*)usbcmd)[0] |= 4;
	// TELL XHCI TO REPORT EVERYTHING
	((unsigned long*)dnctrl)[0] |= 0b1111111111111111;
	// COMMAND RING CONTROLL
	unsigned long zen = (unsigned long)&ring;
	zen = zen<<6;
	((unsigned long*)crcr)[0] = (unsigned long)zen;
	// EVENT RING SIZE
	((unsigned long*)erstsz)[0] |= 1;
	// EVENT RING TABLE SETUP
	unsigned long txrt = (unsigned long)&evts;
	txrt = txrt<<6;
	TRB ert;
	ert.bar1 = txrt;
	ert.bar3 = 16;
	unsigned long tert = (unsigned long)&ert;
	tert = tert<<6;
	//
	// force interrupt to be enabled
	for(int z = 0 ; z < 7 ; z++){
		unsigned long intenaaddr = rtsoff+0x20+(32*z);
		printf("[XHCI] intc %x : %x\n",z,((unsigned long*)intenaaddr)[0]);
		((unsigned long*)intenaaddr)[0] |= 2;
		((unsigned long*)intenaaddr)[0] &= ~1;
		unsigned long inte2aaddr = rtsoff+0x30+(32*z);
		((unsigned long*)inte2aaddr)[0] |= tert;
	}
	// setup the max device sloths
	((unsigned long*)config)[0] |= 0b00000111;
	// DCBAAP
	unsigned long bcbaapt[10];
	unsigned long btc = (unsigned long)&bcbaapt;
	((unsigned long*)bcbaap)[0] = (unsigned long)btc;
	
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
			
			TRB *trb2 = (TRB*)&ring;
			trb2->bar1 = 0;
			trb2->bar2 = 0;
			trb2->bar3 = 0;
			trb2->bar4 = 0b00000000000000000010010000000001;
			
			// doorbel
			unsigned long tingdongaddr = doorbel;
			((unsigned long*)tingdongaddr)[0] = 0;
			
			// wait
			resetTicks();
			while(getTicks()<5);
			
			// result
			TRB* trbres = (TRB*)evts;
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
