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

unsigned long *ic = (unsigned long*)0x54080;
TRB* devring = (TRB*)0x54500;

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
	((unsigned long*)iman_addr)[0] &= ~1;
	printf("[XHCI] ISTS %x \n",((unsigned long*)iman_addr)[0]);
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
	unsigned long ccr = (getBARaddress(bus,slot,function,0x08) & 0b11111111111111111111111100000000) >> 8;
	unsigned long bar = getBARaddress(bus,slot,function,0x10);
	unsigned long sbrn = (getBARaddress(bus,slot,function,0x60) & 0x0000FF);
	printf("[XHCI] Serial Bus Release Number Register %x \n",sbrn);
	printf("[XHCI] Class Code Register %x \n",ccr);
	if(!(ccr==0x0C0330&&(sbrn==0x30||sbrn==0x31))){
		printf("[XHCI] Incompatible device!\n");
		return;
	}
	unsigned long hciversionaddr = bar+2;
	unsigned long hciversion = ((unsigned long*)hciversionaddr)[0];
	printf("[XHCI] hciversion %x \n",hciversion);
	unsigned long capdb = bar+0x14;
	unsigned long hciparamadr = bar+0x04;
	unsigned long hciparam1 = ((unsigned long*)hciparamadr)[0];
	unsigned long portcount = hciparam1>>24;
	unsigned long hccparams1adr = bar+0x10;
	unsigned long hccparams1 = ((unsigned long*)hccparams1adr)[0];
	printf("[XHCI] hccparams1 %x \n",hccparams1);
	if(hccparams1&1){
		printf("[XHCI] has 64-bit Addressing Capability\n");
	}
	if(hccparams1&0xFFFF0000){
		printf("[XHCI] has xHCI Extended Capabilities Pointer ( %x )\n",(hccparams1&0xFFFF0000)>>16);
	}
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
		rtsoff += 0x7C;
		printf("[XHCI] Controller not supported yet!\n");
	}else if(deviceid==0xD){
		printf("[XHCI] QEMU XHCI CONTROLLER\n");
		basebar = bar  + ((unsigned char*)bar)[0];
		printf("[XHCI] Controller not supported yet!\n");return;
	}else if(deviceid==0x15){
		printf("[XHCI] BOCHS XHCI CONTROLLER\n");
		basebar = bar + ((unsigned char*)bar)[0];
	}else if(deviceid==0x1E31){
		printf("[XHCI] VIRTUALBOX XHCI CONTROLLER\n");
		basebar = bar + ((unsigned char*)bar)[0];return;
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
		basebar = bar + ((unsigned char*)bar)[0];
		printf("[XHCI] Controller not supported yet!\n");return;
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
	// TELL XHCI TO REPORT EVERYTHING
//	((unsigned long*)dnctrl)[0] |= 0b1111111111111111;
	
	// setting up interrupter management register
	
	// setting up "Event Ring Segment Table"
	unsigned long rsb1 = 0x1000;
	unsigned long rsb2 = 0x1008;
	((unsigned long*)rsb1)[0] |= 0x41400; 	// pointer to event ring queue
	((unsigned long*)rsb2)[0] |= 16;	// size of ring segment (minimal length)
	
	// setting up "Event Ring Segment Table Size Register (ERSTSZ)"
	unsigned long erstsz_addr = rtsoff + 0x028;
	((unsigned long*)erstsz_addr)[0] |= 1; // keep only 1 open
	
	// setting up "Event Ring Dequeue Pointer Register (ERDP)"
	unsigned long erdp_addr = rtsoff + 0x038;
	((unsigned long*)erdp_addr)[0] = 0x41400; // set addr of event ring dequeue pointer register
	((unsigned long*)erdp_addr)[0] &= ~0b1000; // clear bit 3
	
	// setting up "Event Ring Segment Table Base Address Register (ERSTBA)"
	unsigned long erstba_addr = rtsoff + 0x030;
	((unsigned long*)erstba_addr)[0] = 0x1000; 	// table at 0x1000 for now
	((unsigned long*)erstba_addr)[1] = 0x0; 	// sending 0 to make sure...
	
	// setting up "Command Ring Control Register (CRCR)"
	unsigned long bse = 0x1500;
	unsigned long bse1 = bse<<6;
	((unsigned long*)crcr)[0] |= bse1;
	((unsigned long*)crcr)[1] = 0;
	
	// DCBAAP
	unsigned long btc = 0x6000;
	((unsigned long*)bcbaap)[0] |= (unsigned long)btc;
	((unsigned long*)bcbaap)[1] = 0;
	
	// setting first interrupt enabled.
	if(0){
		unsigned long iman_addr = rtsoff + 0x020;
		((unsigned long*)iman_addr)[0] |= 0b10; // Interrupt Enable (IE) – RW
	}
	// TELL XHCI TO USE INTERRUPTS
	((unsigned long*)usbcmd)[0] |= 4;
	
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
		
			// detecting speed....
			unsigned long speed = (val >> 10) & 0b000000000000000000000000000111;
			printf("[XHCI] Port %x devicespeed is %x \n",i,speed);
			
			//
			// Device Slot Assignment
			//
			printf("[XHCI] Obtaining device slot...\n");
			
			// setting up TRB
			
			TRB* trb2 = ((TRB*)0x54000);
			trb2->bar1 = 0;
			trb2->bar2 = 0;
			trb2->bar3 = 0;
			trb2->bar4 = 0b00000000000000000010010000000000;
			
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
			unsigned long iman_addr = rtsoff + 0x020;
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			// RESULTS
			TRB *trbres = ((TRB*)0x41400);
			unsigned char assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
			unsigned char completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
			printf("[XHCI] Completion event arived. slot=%x code=%x \n",assignedSloth,completioncode);
			if(completioncode!=1){
				printf("[XHCI] Panic: completioncode != 1 \n");
				for(;;);
			}
			
			//
			// Device Slot Initialisation
			//
			
			printf("[XHCI] Device Slot Initialisation \n");
			unsigned long bse = (unsigned long)malloc(0x420);
			((unsigned long*)0x6000)[(assignedSloth*2)+0] 	= bse;
			((unsigned long*)0x6000)[(assignedSloth*2)+1] 	= 0;
			
			unsigned long speedint = (speed << 20) & 0b1111;
			// Input Control Context
			((unsigned long*)0x54080)[0] = 0;
			((unsigned long*)0x54084)[0] = 3;
			((unsigned long*)0x54088)[0] = 0;
			((unsigned long*)0x5408C)[0] = 0;
			
			// Slot(h) Context
			((unsigned long*)0x540C0)[0] = 0b00001000000000000000000000000000;
			((unsigned long*)0x540C4)[0] = 0b00000000000000000000000000000000 | (assignedSloth<<16);
			((unsigned long*)0x540C8)[0] = 0;//0b00000000010000000000000000000000;
			((unsigned long*)0x540CC)[0] = 0;
			
			// Endpoint Context
			((unsigned long*)0x54100)[0] = 1;
			((unsigned long*)0x54104)[0] = 0b00000000000000010000000000100110;//0;
			((unsigned long*)0x54108)[0] = 0x54500 | 1;//0;
			((unsigned long*)0x5410C)[0] = 0;
			
			// Address Device Command
			trb = ((TRB*)0x54010);
			trb->bar1 = 0x54080;
			trb->bar2 = 0;
			trb->bar3 = 0;
			trb->bar4 = 0b00000000000000000010110000000000;
			unsigned long longsloth = assignedSloth;
			longsloth = longsloth << 24;
			trb->bar4 |= longsloth;
			
			// stop codon
			TRB *trb3 = ((TRB*)0x54020);
			trb3->bar1 = 0;
			trb3->bar2 = 0;
			trb3->bar3 = 0;
			trb3->bar4 = 1;
			
			// doorbell
			((unsigned long*)tingdongaddr)[0] = 0;
			
			// wait
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			// RESULTS
			trbres = ((TRB*)0x41410);
			assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
			completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
			printf("[XHCI] Completion event arived. slot=%x code=%x \n",assignedSloth,completioncode);
			if(completioncode!=1)
			{
				printf("[XHCI] Panic: completioncode != 1 \n"); // returns 0x04
				for(;;);
			}
			
			
			printf("[XHCI] Configure Endpoint Command\n");
			trb3 = ((TRB*)0x54020);
			trb3->bar1 = 0x54080;
			trb3->bar2 = 0;
			trb3->bar3 = 0;
			trb3->bar4 = 0b00000000000000000011000000000000;
			longsloth = assignedSloth;
			longsloth = longsloth << 24;
			trb3->bar4 |= longsloth;
			
			TRB *trb4 = ((TRB*)0x54030);
			trb4->bar1 = 0;
			trb4->bar2 = 0;
			trb4->bar3 = 0;
			trb4->bar4 = 1;
			
			((unsigned long*)tingdongaddr)[0] = 0;
			
			// wait
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			// RESULTS
			trbres = ((TRB*)0x41420);
			assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
			completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
			printf("[XHCI] Completion event arived. slot=%x code=%x \n",assignedSloth,completioncode);
			if(completioncode!=1)
			{
				printf("[XHCI] Panic: completioncode != 1 \n");
				for(;;);
			}
			
			//
			// GET DEVICE DESCRIPTOR
			// trb-type=2
			// trt=3
			// transferlength=8
			// IOC=0
			// IDT=1
			// reqtype= 0x80
			// req=6
			// wValue=0100
			// wIndex=0
			// wLength=0
			//
			
			unsigned char devicedescriptor[8];
			TRB *dc1 = ((TRB*)0x54500);
			dc1->bar1 = 0b00000000000001000000011010000000;
			dc1->bar2 = 0b00000000000000000000000000000000;
			dc1->bar3 = 0b00000000010000000000000000001000;
			dc1->bar4 = 0b00000000000000011000100001000000;
			
			
			((unsigned long*)tingdongaddr)[assignedSloth] = 0;
			
			printf("[XHCI] Device initialised succesfully\n");
			for(;;);
		}
	}
	
	if(((unsigned long*)crcr)[0]==0x8){
		printf("[XHCI] circulair command ring is running\n");
	}
	printf("[XHCI] All finished!\n");
	for(;;);
}
