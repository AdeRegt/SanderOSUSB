#include "../kernel.h"
#define XHCI_DEVICE_BOCHS 0x15
#define XHCI_SPEED_FULL   1
#define XHCI_SPEED_LOW    2
#define XHCI_SPEED_HI     3
#define XHCI_SPEED_SUPER  4

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
unsigned long deviceid = 0;
unsigned long iman_addr= 0;

TRB event_ring_queue[20] __attribute__ ((aligned (0x100))); 
TRB command_ring_control[20] __attribute__ ((aligned (0x100)));
unsigned long command_ring_offset = 0;
unsigned long event_ring_offset = 0;

int xhci_set_address(unsigned long assignedSloth,unsigned long* t,unsigned char bsr){
	// Address Device Command BSR1
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb->bar1 = (unsigned long)t&0xFFFFFFF0;
	trb->bar2 = 0;
	trb->bar3 = 0;
	trb->bar4 = 0;
	if(deviceid!=XHCI_DEVICE_BOCHS){
		trb->bar4 |= 1; // set cycle bit
	}
	if(bsr){
		trb->bar4 |= (1<<9); // set bsr bit
	}
	trb->bar4 |= (11<<10); // trb type
	trb->bar4 |= (assignedSloth<<24); // assigned sloth
	
	command_ring_offset += 0x10;
	
	// stop codon
	TRB *trb6 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb6->bar1 = 0;
	trb6->bar2 = 0;
	trb6->bar3 = 0;
	if(deviceid!=XHCI_DEVICE_BOCHS){
		trb6->bar4 = 0;
	}else{
		trb6->bar4 = 1;
	}
	
	// doorbell
	((unsigned long*)doorbel)[0] = 0;
	
	// wait
	while(1){
		unsigned long r = ((unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
	}
	
	// RESULTS
	TRB* trbres2 = ((TRB*)((unsigned long)(&event_ring_queue)+event_ring_offset));
	unsigned long completioncode2 = (trbres2->bar3 & 0b111111100000000000000000000000) >> 24;
	
	event_ring_offset += 0x10;
	return completioncode2;
}

int xhci_enable_slot(){
	TRB* trb2 = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb2->bar1 = 0;
	trb2->bar2 = 0;
	trb2->bar3 = 0;
	if(deviceid!=XHCI_DEVICE_BOCHS){
		trb2->bar4 = 0b00000000000000000010010000000001;
	}else{
		trb2->bar4 = 0b00000000000000000010010000000000;
	}
	
	command_ring_offset += 0x10;
	
	TRB* trb = ((TRB*)((unsigned long)(&command_ring_control)+command_ring_offset));
	trb->bar1 = 0;
	trb->bar2 = 0;
	trb->bar3 = 0;
	if(deviceid!=XHCI_DEVICE_BOCHS){
		trb->bar4 = 0;
	}else{
		trb->bar4 = 1;
	}
	
	((unsigned long*)doorbel)[0] = 0;
	
	while(1){
		unsigned long r = ((unsigned long*)iman_addr)[0];
		if(r&1){
			break;
		}
	}
	
	TRB *trbres = ((TRB*)((unsigned long)(&event_ring_queue)+event_ring_offset));
	unsigned char assignedSloth = (trbres->bar4 & 0b111111100000000000000000000000) >> 24;
	unsigned char completioncode = (trbres->bar3 & 0b111111100000000000000000000000) >> 24;
	if(completioncode!=1){
		return 0;
	}
	
	event_ring_offset += 0x10;
	return assignedSloth;
}

void irq_xhci(){
	unsigned long xhci_usbsts = ((unsigned long*)usbsts)[0];
	if(xhci_usbsts&4){
		printf("[XHCI] Host system error interrupt\n");
	}
//	if(xhci_usbsts&8){
//		printf("[XHCI] Event interrupt\n");
//	}
//	if(xhci_usbsts&0x10){
//		printf("[XHCI] Port interrupt\n");
//	}
//	unsigned long iman_addr = rtsoff + 0x020;
//	((unsigned long*)iman_addr)[0] &= ~1;
//	printf("[XHCI] ISTS %x \n",((unsigned long*)iman_addr)[0]);
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
	deviceid = (getBARaddress(bus,slot,function,0) & 0xFFFF0000) >> 16;
	unsigned long ccr = (getBARaddress(bus,slot,function,0x08) & 0b11111111111111111111111100000000) >> 8;
	unsigned long bar = getBARaddress(bus,slot,function,0x10);
	unsigned long sbrn = (getBARaddress(bus,slot,function,0x60) & 0x0000FF);
	
	printf("[XHCI] START=%x \n",bar);
	while(1){
		if((bar&0xFF)>0x80){
			bar++;
		}else{
			bar--;
		}
		if((bar&0xFF)==00){
			break;
		}
	}
	printf("[XHCI] HALT=%x SEGM=%x \n",bar,((unsigned char*)bar)[0]);
	
//
// Calculating base address
	basebar = bar+((unsigned char*)bar)[0];
	if(deviceid==0x22B5){
		printf("[XHCI] INTELL XHCI CONTROLLER\n");
	}else if(deviceid==0xD){
		printf("[XHCI] QEMU XHCI CONTROLLER\n");
	}else if(deviceid==XHCI_DEVICE_BOCHS){
		printf("[XHCI] BOCHS XHCI CONTROLLER\n");
	}else if(deviceid==0x1E31){
		printf("[XHCI] VIRTUALBOX XHCI CONTROLLER\n");
	}else{
		printf("[XHCI] UNKNOWN XHCI CONTROLLER %x \n",deviceid);
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
	rtsoff = bar+(((unsigned long*)rtsoffa)[0]&0xFFFFFFE0);
	iman_addr = rtsoff + 0x020;
	
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
	printf("[XHCI] Setting up Event Ring Segment Table\n");
	unsigned long rsb1[20]  __attribute__ ((aligned (0x100)));
	unsigned long rsb2 = ((unsigned long)&rsb1)+4;
	unsigned long rsb3 = ((unsigned long)&rsb1)+8;
	
	((unsigned long*)&rsb1)[0] = ((unsigned long)&event_ring_queue); 	// pointer to event ring queue 0x41400
	((unsigned long*)rsb2)[0] = 0;
	((unsigned long*)rsb3)[0] |= 16;	// size of ring segment (minimal length)
	
	// setting up "Event Ring Segment Table Size Register (ERSTSZ)"
	printf("[XHCI] Setting up Event Ring Segment Size Register\n");
	unsigned long erstsz_addr = rtsoff + 0x028;
	((unsigned long*)erstsz_addr)[0] |= 1; // keep only 1 open
	
	// setting up "Event Ring Dequeue Pointer Register (ERDP)"
	printf("[XHCI] Setting up Event Ring Dequeue Pointer Register\n");
	unsigned long erdp_addr = rtsoff + 0x038;
	((unsigned long*)erdp_addr)[0] = ((unsigned long)&event_ring_queue); // set addr of event ring dequeue pointer register
	((unsigned long*)erdp_addr)[0] &= ~0b1000; // clear bit 3
	((unsigned long*)erdp_addr)[1] = 0; // clear 64bit
	
	// setting up "Event Ring Segment Table Base Address Register (ERSTBA)"
	printf("[XHCI] Setting up Event Ring Segment Table Bse Address Register\n");
	unsigned long erstba_addr = rtsoff + 0x030;
	((unsigned long*)erstba_addr)[0] = (unsigned long)&rsb1; 	// table at 0x1000 for now
	((unsigned long*)erstba_addr)[1] = 0; 	// sending 0 to make sure...
	
	// setting up "Command Ring Control Register (CRCR)"
	printf("[XHCI] Setting up Command Ring Control Register\n");
	((unsigned long*)crcr)[0] |= ((unsigned long)&command_ring_control);
	((unsigned long*)crcr)[1] = 0;
	
	// DCBAAP
	printf("[XHCI] Setting up DCBAAP\n");
	unsigned long btc[20] __attribute__ ((aligned (0x100)));
	((unsigned long*)bcbaap)[0] |= (unsigned long)&btc;
	((unsigned long*)bcbaap)[1] = 0;
	
	// setting first interrupt enabled.
	if(0){
		printf("[XHCI] Setting up First Interrupter\n");
		unsigned long iman_addr = rtsoff + 0x020;
		((unsigned long*)iman_addr)[0] |= 0b10; // Interrupt Enable (IE) – RW
	}
	// TELL XHCI TO USE INTERRUPTS
	printf("[XHCI] Use interrupts\n");
	((unsigned long*)usbcmd)[0] |= 4;
	
	printf("[XHCI] Wait 5s\n");
	resetTicks();
	while(getTicks()<5);
//
// Start system
	printf("[XHCI] Run!\n");
	((unsigned long*)usbcmd)[0] |= 1;
	
	printf("[XHCI] Wait 5s\n");
	resetTicks();
	while(getTicks()<5);
	xhci_usbcmd = ((unsigned long*)usbcmd)[0];
	xhci_usbsts = ((unsigned long*)usbsts)[0];
	printf("[XHCI] System up and running with USBCMD %x and USBSTS %x \n",xhci_usbcmd,xhci_usbsts);
	
	printf("[XHCI] Checking if portchange happened\n");
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
			unsigned long devicespeed = 0;// 8 64 512
			if(speed==XHCI_SPEED_SUPER){
				printf("[XHCI] Port %x : port is a superspeed port\n",i);
				devicespeed = 512;
			}else if(speed==XHCI_SPEED_HI){
				printf("[XHCI] Port %x : port is a highspeed port\n",i);
				devicespeed = 64;
			}else if(speed==XHCI_SPEED_FULL){
				printf("[XHCI] Port %x : port is a fullspeed port\n",i);
				devicespeed = 64;
			}else if(speed==XHCI_SPEED_LOW){
				printf("[XHCI] Port %x : port is a lowspeed port\n",i);
				devicespeed = 8;
			}
			
			//
			//
			// Device Slot Assignment
			//
			//
			
			printf("[XHCI] Port %x : Obtaining device slot...\n",i);
			unsigned long assignedSloth = xhci_enable_slot();
			if(assignedSloth==0){
				printf("[XHCI] Port %x : Assignation of device slot failed!\n",i);
				continue;
			}
			
			//
			//
			// Device Slot Initialisation
			//
			//
			
			printf("[XHCI] Port %x : Device Slot Initialisation BSR1 \n",i);
			
			TRB local_ring_control[20] __attribute__ ((aligned (0x100)));
	
			printf("[XHCI] Port %x : Setting up DCBAAP for port \n",i);
			unsigned long bse = (unsigned long)malloc(0x420);
			btc[(assignedSloth*2)+0] 	= bse;
			btc[(assignedSloth*2)+1] 	= 0;
			
			printf("[XHCI] Port %x : Setting up input controll\n",i);
			unsigned long *t = (unsigned long*)malloc(0x54);
			
			printf("[XHCI] Port %x : Setting up Input Controll Context\n",i);
			// Input Control Context
			t[0x00] = 0;
			t[0x01] = 0;
			t[0x02] = 0;
			t[0x03] = 0;
			
			t[0x01] |= 0b00000000000000000000000000000011 ; // enabling A0 and A1
			
			printf("[XHCI] Port %x : Setting up Slot Context\n",i);
			// Slot(h) Context
			t[0x10] = 0;
			t[0x11] = 0;
			t[0x12] = 0;
			t[0x13] = 0;
			
			t[0x11] |= (1<<16); // Root hub port number
			t[0x10] |= 0; // route string
			t[0x10] |= (1<<27); // context entries 
			
			printf("[XHCI] Port %x : Setting up Endpoint Context\n",i);
			// Endpoint Context
			t[0x20] = 0;
			t[0x21] = 0;
			t[0x22] = 0;
			t[0x23] = 0;
			
			t[0x21] |= (4<<3); // set ep_type to controll
			t[0x21] |= (devicespeed<<16); // set max packet size
			t[0x21] |= (0<<8); // max burst size
			t[0x22] |= (unsigned long)&local_ring_control; // TR dequeue pointer
			t[0x22] |= 1; // dequeue cycle state
			t[0x20] |= (0<<16); // set interval 
			t[0x20] |= (0<<10); // set max primairy streams
			t[0x20] |= (0<<8); // set mult
			t[0x21] |= (3<<1); // set CErr
			
			//
			//
			// SETADDRESS commando
			//
			//
			
			
			printf("[XHCI] Port %x : Obtaining SETADDRESS(BSR=1)\n",i);
			int sares = xhci_set_address(assignedSloth,t,1);
			if(sares!=1){
				printf("[XHCI] Port %x : Assignation of device slot address failed with %x !\n",i,sares);
				continue;
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
			
			
			((unsigned long*)doorbel)[assignedSloth] = 1;
			
			while(1){
				unsigned long r = ((unsigned long*)iman_addr)[0];
				if(r&1){
					break;
				}
			}
			
			printf("[XHCI] Port %x : devdesc %x %x %x %x %x %x %x %x \n",i,devicedescriptor[0],devicedescriptor[1],devicedescriptor[2],devicedescriptor[3],devicedescriptor[4],devicedescriptor[5],devicedescriptor[6],devicedescriptor[7]);
			printf("[XHCI] Port %x : deviceclass=%x \n",i,devicedescriptor[4]);
			if(devicedescriptor[4]==0){
				printf("[XHCI] Port %x : Deviceclass cannot be 0! \n",i);
			}else{
				printf("[XHCI] Port %x : Device initialised succesfully\n",i);
			}
			sleep(10000);
		}else{
			printf("[XHCI] Port %x : No device attached!\n",i);
		}
	}
	
	if(((unsigned long*)crcr)[0]==0x8){
		printf("[XHCI] circulair command ring is running\n");
	}
	printf("[XHCI] All finished!\n");for(;;);
}
