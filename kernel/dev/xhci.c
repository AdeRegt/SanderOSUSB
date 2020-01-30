#include "../kernel.h"
#define XHCI_DEVICE_BOCHS 0x15

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
	
	signed long offset = 0;
	
	if(deviceid!=0x22B5){
		while(1){
			if((bar&0xFF)>0x80){bar++;offset--;}else{bar--;offset++;}
			if((bar&0xFF)==00){
				break;
			}
		}
	}
//
// Calculating base address
	if(deviceid==0x22B5){
		printf("[XHCI] INTELL XHCI CONTROLLER\n");
		basebar = bar+0x7C;
		printf("[XHCI] Controller not supported yet!\n");
	}else if(deviceid==0xD){
		printf("[XHCI] QEMU XHCI CONTROLLER\n");
		basebar = bar  + ((unsigned char*)bar)[0];
		printf("[XHCI] Controller not supported yet!\n");
	}else if(deviceid==XHCI_DEVICE_BOCHS){
		printf("[XHCI] BOCHS XHCI CONTROLLER\n");
		basebar = bar + ((unsigned char*)bar)[0];
	}else if(deviceid==0x1E31){
		printf("[XHCI] VIRTUALBOX XHCI CONTROLLER\n");
		basebar = bar + ((unsigned char*)bar)[0];
		return;
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
		basebar = bar + ((unsigned char*)bar)[0];
		printf("[XHCI] Controller not supported yet!\n");
		return;
	}
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
	
	while(1){
		if((rtsoff&0xFF)>0x80){rtsoff++;}else{rtsoff--;}
		if((rtsoff&0xFF)==00){
			break;
		}
	}
	unsigned long rtsoffa = bar+0x18;
	rtsoff = bar+(((unsigned long*)rtsoffa)[0]);
	
	printf("[XHCI] Runtime offset %x (BAR: %x)\n",rtsoff,bar);
	printf("[XHCI] portcount %x \n",portcount);
	doorbel = bar+((unsigned long*)capdb)[0];
	printf("[XHCI] doorbell offset %x \n",doorbel);
	
	printf("[XHCI] basebar=%x \n",basebar);
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
	TRB event_ring_queue[20] __attribute__ ((aligned (0x100))); 
	unsigned long rsb1[20]  __attribute__ ((aligned (0x100)));
	unsigned long rsb2 = ((unsigned long)&rsb1)+4;
	unsigned long rsb3 = ((unsigned long)&rsb1)+8;
	
	((unsigned long*)&rsb1)[0] = ((unsigned long)&event_ring_queue); 	// pointer to event ring queue 0x41400
	((unsigned long*)rsb2)[0] = 0;
	((unsigned long*)rsb3)[0] |= 16;	// size of ring segment (minimal length)
	
	// setting up "Event Ring Segment Table Size Register (ERSTSZ)"
	unsigned long erstsz_addr = rtsoff + 0x028;
	((unsigned long*)erstsz_addr)[0] |= 1; // keep only 1 open
	
	// setting up "Event Ring Dequeue Pointer Register (ERDP)"
	unsigned long erdp_addr = rtsoff + 0x038;
	((unsigned long*)erdp_addr)[0] = ((unsigned long)&event_ring_queue); // set addr of event ring dequeue pointer register
	((unsigned long*)erdp_addr)[0] &= ~0b1000; // clear bit 3
	((unsigned long*)erdp_addr)[1] = 0; // clear 64bit
	
	// setting up "Event Ring Segment Table Base Address Register (ERSTBA)"
	unsigned long erstba_addr = rtsoff + 0x030;
	((unsigned long*)erstba_addr)[0] = (unsigned long)&rsb1; 	// table at 0x1000 for now
	((unsigned long*)erstba_addr)[1] = 0; 	// sending 0 to make sure...
	
	// setting up "Command Ring Control Register (CRCR)"
	TRB command_ring_control[20] __attribute__ ((aligned (0x100)));
	((unsigned long*)crcr)[0] |= ((unsigned long)&command_ring_control);
	((unsigned long*)crcr)[1] = 0;
	
	// DCBAAP
	unsigned long btc[20] __attribute__ ((aligned (0x100)));
	((unsigned long*)bcbaap)[0] |= (unsigned long)&btc;
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
	for(unsigned int i = 0 ; i < 10 ; i++){//5
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
			printf("[XHCI] Port %x : devicespeed is %x \n",i,speed);
			if(speed==3){
				printf("[XHCI] Port %x : device is highspeed\n",i);
			}
			
			//
			// Device Slot Assignment
			//
			printf("[XHCI] Port %x : Obtaining device slot...\n",i);
			
			// setting up TRB
			
			TRB* trb2 = ((TRB*)((unsigned long)&command_ring_control));
			trb2->bar1 = 0;
			trb2->bar2 = 0;
			trb2->bar3 = 0;
			if(deviceid!=XHCI_DEVICE_BOCHS){
				trb2->bar4 = 0b00000000000000000010010000000001;
			}else{
				trb2->bar4 = 0b00000000000000000010010000000000;
			}
			
			// STOP CODON
			TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+0x10));
			trb->bar1 = 0;
			trb->bar2 = 0;
			trb->bar3 = 0;
			if(deviceid!=XHCI_DEVICE_BOCHS){
				trb->bar4 = 0;
			}else{
				trb->bar4 = 1;
			}
			
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
			TRB *trbres = ((TRB*)((unsigned long)&event_ring_queue));
			unsigned char assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
			unsigned char completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
			printf("[XHCI] Port %x : Completion event arived. slot=%x code=%x \n",i,assignedSloth,completioncode);
			if(completioncode!=1){
				printf("[XHCI] Port %x : Panic: completioncode != 1 \n",i);
				for(;;);
			}
			
			//
			// Device Slot Initialisation
			//
			
			printf("[XHCI] Port %x : Device Slot Initialisation \n",i);
			
			TRB local_ring_control[20] __attribute__ ((aligned (0x100)));
	
			unsigned long bse = (unsigned long)malloc(0x420);
			btc[(assignedSloth*2)+0] 	= bse;
			btc[(assignedSloth*2)+1] 	= 0;
			
			//unsigned long speedint = (speed << 20) & 0b1111;
			unsigned long *t = (unsigned long*)malloc(0x54);//[0x54];
			// Input Control Context
			t[0x00] = 0;
			t[0x01] = 3;
			t[0x02] = 0;
			t[0x03] = 0;
			
			// Slot(h) Context
			t[0x10] = 0b00001000000110000000000000000000;
			t[0x11] = 0b00000000000000000000000000000000 | (assignedSloth<<16);
			t[0x12] = 0;//0b00000000010000000000000000000000;
			t[0x13] = 0;
			
			// Endpoint Context
			t[0x20] = 1;
			t[0x21] = 0b00000000001000000000000000100110;//0b00000000000000010000000000100110;//0;
			t[0x22] = ((unsigned long)&local_ring_control) | 1;//0;
			t[0x23] = 0;
			
			// Address Device Command
			trb = ((TRB*)((unsigned long)(&command_ring_control)+0x10));
			trb->bar1 = (unsigned long)t;
			trb->bar2 = 0;
			trb->bar3 = 0;
			if(deviceid!=XHCI_DEVICE_BOCHS){
				trb->bar4 = 0b00000000000000000010111000000001;
			}else{
				trb->bar4 = 0b00000000000000000010111000000000;
			}
			unsigned long longsloth = assignedSloth;
			longsloth = longsloth << 24;
			trb->bar4 |= longsloth;
			
			// stop codon
			TRB *trb3 = ((TRB*)((unsigned long)(&command_ring_control)+0x20));
			trb3->bar1 = 0;
			trb3->bar2 = 0;
			trb3->bar3 = 0;
			if(deviceid!=XHCI_DEVICE_BOCHS){
				trb3->bar4 = 0;
			}else{
				trb3->bar4 = 1;
			}
			
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
			trbres = ((TRB*)((unsigned long)(&event_ring_queue)+0x10));
			assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
			completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
			printf("[XHCI] Port %x : Completion event arived. slot=%x code=%x \n",i,assignedSloth,completioncode);
			if(completioncode!=1)
			{
				printf("[XHCI] Port %x : Panic: completioncode != 1 \n",i); // returns 0x04
				for(;;);
			}
			
			
			printf("[XHCI] Port %x : Configure Endpoint Command\n",i);
			trb3 = ((TRB*)((unsigned long)(&command_ring_control)+0x20));
			trb3->bar1 = (unsigned long)t;
			trb3->bar2 = 0;
			trb3->bar3 = 0;
			trb3->bar4 = 0b00000000000000000011000000000000;
			longsloth = assignedSloth;
			longsloth = longsloth << 24;
			trb3->bar4 |= longsloth;
			
			TRB *trb4 = ((TRB*)((unsigned long)(&command_ring_control)+0x30));
			trb4->bar1 = 0;
			trb4->bar2 = 0;
			trb4->bar3 = 0;
			if(deviceid!=XHCI_DEVICE_BOCHS){
				trb4->bar4 = 0;
			}else{
				trb4->bar4 = 1;
			}
			
			((unsigned long*)tingdongaddr)[0] = 0;
			
			// wait
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			// RESULTS
			trbres = ((TRB*)((unsigned long)(&event_ring_queue)+0x20));
			assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
			completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
			printf("[XHCI] Port %x : Completion event arived. slot=%x code=%x \n",i,assignedSloth,completioncode);
			if(completioncode!=1)
			{
				printf("[XHCI] Port %x : Panic: completioncode != 1 \n",i);
				for(;;);
			}
			
			printf("[XHCI] Port %x : GET DEVICE DESCRIPTOR\n",i);
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
			
			unsigned char *devicedescriptor = (unsigned char*)malloc(8);
			printf("[XHCI] devicedescriptor located at %x \n",(unsigned long)devicedescriptor);
			TRB *dc1 = ((TRB*)((unsigned long)&local_ring_control));
			dc1->bar1 = 0b00000000000001000000011010000000;
			dc1->bar2 = 0b00000000000000000000000000000000;
			dc1->bar3 = 0b00000000010000000000000000001000;
			dc1->bar4 = 0b00000000000000110000100001000001;
			
			// single date stage
			// TRB Type = Data Stage TRB.
			// X Direction (DIR) = ‘1’.
			// X TRB Transfer Length = 8.
			// X Chain bit (CH) = 0.
			// X Interrupt On Completion (IOC) = 0.
			// X Immediate Data (IDT) = 0.
			// X Data Buffer Pointer = The address of the Device Descriptor receive buffer.
			// X Cycle bit = Current Producer Cycle State.
			TRB *dc2 = ((TRB*)((unsigned long)(&local_ring_control)+0x10));
			dc2->bar1 = (unsigned long)devicedescriptor;
			dc2->bar2 = 0b00000000000000000000000000000000;
			dc2->bar3 = 0b00000000000000000000000000001000;
			dc2->bar4 = 0b00000000000000010000110000000001;
			
			TRB *dc3 = ((TRB*)((unsigned long)(&local_ring_control)+0x20));
			dc3->bar1 = 0;
			dc3->bar2 = 0;
			dc3->bar3 = 0;
			dc3->bar4 = 0;
			
			
			((unsigned long*)tingdongaddr)[assignedSloth] = 1;
			
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			printf("[XHCI] Port %x : devdesc %x %x %x %x %x %x %x %x \n",i,devicedescriptor[0],devicedescriptor[1],devicedescriptor[2],devicedescriptor[3],devicedescriptor[4],devicedescriptor[5],devicedescriptor[6],devicedescriptor[7]);
			printf("[XHCI] Port %x : deviceclass=%x \n",i,devicedescriptor[4]);
			printf("[XHCI] Port %x : Device initialised succesfully\n",i);
			sleep(10000);
		}
	}
	
	if(((unsigned long*)crcr)[0]==0x8){
		printf("[XHCI] circulair command ring is running\n");
	}
	printf("[XHCI] All finished!\n");for(;;);
}
