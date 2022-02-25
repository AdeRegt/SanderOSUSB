// nieuw
#include "../kernel.h"

#define MAX_RING_SIZE 16
#define TRB_TYPE_ENABLE_SLOT 9
#define TRB_TYPE_SET_ADDRESS 11

struct XHCI_64bit {
    unsigned long ptr_low;
    unsigned long ptr_high;
} __attribute__((packed));

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

struct XHCI_OperationalPortRegisters{
	unsigned long PORTSC;
	unsigned long PORTPMSC;
	unsigned long PORTLI;
	unsigned long PORTHLPMC;
}__attribute__((packed));

struct XHCI_RuntimeInteruptRegisters{
	unsigned long IMAN;
	unsigned long IMOD;
	unsigned long ERSTSZ;
	unsigned long RsvdP;
	struct XHCI_64bit ERSTBA;
	struct XHCI_64bit ERDP;
}__attribute__((packed));

struct XHCI_RuntimeRegisters{
	unsigned long MFINDEX;
	unsigned char reserved[0x1C];
	struct XHCI_RuntimeInteruptRegisters interupts[0x20];
}__attribute__((packed));

struct XHCI_TRB{
	unsigned long arg1;
	unsigned long arg2;
	unsigned long arg3;
	unsigned long arg4;
}__attribute__((packed));

#define WANTED_RING_SIZE (sizeof(struct XHCI_TRB)*MAX_RING_SIZE)

struct XHCI_InputControlContext{
	unsigned long D;
	unsigned long A;
	unsigned long reserved[5];
	unsigned char configuration_value;
	unsigned char interface_number;
	unsigned char alternate_setting;
	unsigned char reserved2;
}__attribute__((packed));

struct XHCI_Context{
	unsigned long arg1;
	unsigned long arg2;
	unsigned long arg3;
	unsigned long arg4;
	unsigned long arg5;
	unsigned long arg6;
	unsigned long arg7;
	unsigned long arg8;
}__attribute__((packed));

struct XHCI_RingManager{
	int pointer;
	char cs;
	int event_count;
	volatile struct XHCI_TRB *ring;
}__attribute__((packed));

void xhci_write_long(unsigned long location,unsigned long value){
	printf("[XHCI] Writing %x to %x \n",value,location);
	((unsigned long*)location)[0] = value;
}

struct XHCI_CapibilityRegisters *capability;
volatile struct XHCI_OperationalRegisters *operational;
volatile struct XHCI_OperationalPortRegisters *operational_port;
volatile struct XHCI_RuntimeRegisters *runtime;
struct XHCI_64bit *DCBAAP;
volatile struct XHCI_TRB *command_ring;
volatile struct XHCI_TRB *event_ring;
volatile unsigned long *doorbell;

int ring_queue_index = 0;

extern void xhciirq();

void irq_xhci(){
	printf("[XHCI] Interrupt %x \n", operational->USBSTS);

	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

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

void xhci_dump_event_ring(){
	printf("=========EVENT=RING=============\n");
	for(int i = 0 ; i < MAX_RING_SIZE ; i++){
		unsigned long bar1 = event_ring[i].arg1;
		unsigned long bar2 = event_ring[i].arg2;
		unsigned long bar3 = event_ring[i].arg3;
		unsigned long bar4 = event_ring[i].arg4;
		if(bar1||bar2||bar3||bar4){
			printf("[XHCI] Ring element %x : %x %x %x %x \n",i,bar1,bar2,bar3,bar4);
			printf(" - Address %x %x | Transferlength %x | Completioncode %x \n",bar1,bar2,bar3 & 0b111111111111111111111111,(bar3>>24)&&0b11111111);
			printf(" - Cyclebit %x | EventData %x | TRBType %x | EndpointID %x | SlotID %x \n", bar4 & 1 , (bar4>>2) & 1, (bar4>>10) & 0b11111 , (bar4>>16) & 0b11111 , (bar4>>24) & 0b11111111 );
		}
	}
	printf("[XHCI] USBSTS %x \n",operational->USBSTS);
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
	xhci_wait_for_hostcontroller();

	//
	// Set CONFIG
	unsigned char MaxSlotsEn = (capability->HCCPARAMS1 >> 24) & 0xFF;
	xhci_write_long((unsigned long)&operational->CONFIG,MaxSlotsEn);

	//
	// Set table
	event_ring = (volatile struct XHCI_TRB *)malloc_align(WANTED_RING_SIZE,0xFF);
	unsigned long event_ring_descriptor_table[20] __attribute__ ((aligned (0x100)));
	xhci_write_long((unsigned long)&event_ring_descriptor_table[0],(unsigned long) event_ring);
	xhci_write_long((unsigned long)&event_ring_descriptor_table[1],(unsigned long) 0);
	xhci_write_long((unsigned long)&event_ring_descriptor_table[2],(unsigned long) MAX_RING_SIZE);

	xhci_write_long((unsigned long)&runtime->interupts[0].ERSTSZ, MAX_RING_SIZE);
	xhci_write_long((unsigned long)&runtime->interupts[0].ERDP.ptr_low,(unsigned long) event_ring);
	xhci_write_long((unsigned long)&runtime->interupts[0].ERDP.ptr_high,(unsigned long) 0);

	xhci_write_long((unsigned long)&runtime->interupts[0].ERSTBA.ptr_low,(unsigned long) &event_ring_descriptor_table);
	xhci_write_long((unsigned long)&runtime->interupts[0].ERSTBA.ptr_high,(unsigned long) 0);

	//
	// CRCR
	command_ring = malloc_align(WANTED_RING_SIZE,0xFF);
	xhci_write_long((unsigned long)&operational->CRCR1,(unsigned long)command_ring | 1);
	xhci_write_long((unsigned long)&operational->CRCR2,0);

	//
	// Set DCBAAP
	DCBAAP = (struct XHCI_64bit*) malloc_align(MaxSlotsEn*(sizeof(unsigned long)*2),0xFF);
	xhci_write_long((unsigned long)&operational->DCBAAP1,(unsigned long)DCBAAP);
	xhci_write_long((unsigned long)&operational->DCBAAP2,0);

	if(operational->USBSTS&4){ // serious error bit set
		printf("[XHCI] An serious error occured\n");
		for(;;);
	}

	//
	// SCATCHPAD BUFFER
	unsigned long spb = (capability->HCSPARAMS2 >> 21 & 0x1f) << 5 | capability->HCSPARAMS2 >> 27;
	if(spb){
		printf("[XHCI] SCRATCHPAD buffer %x \n",spb);
		unsigned long *scratchpad = (unsigned long*) malloc((spb*400000)*2);
		struct XHCI_64bit *spx = (struct XHCI_64bit*) malloc(spb*sizeof(struct XHCI_64bit));
		for(unsigned long i = 0 ; i < spb ; i++){
			spx[i].ptr_low 		= (unsigned long)(scratchpad + (i*400000));
			spx[i].ptr_high 	= 0;
		}
		DCBAAP[0].ptr_low = (unsigned long) scratchpad;
		DCBAAP[0].ptr_high = 0;
	}

	xhci_wait_for_hostcontroller();

	operational->USBCMD |= 0b101;

	// wait before the ring is running
	while(operational->USBSTS & 1);

	xhci_wait_for_hostcontroller();

	printf("[XHCI] Basic setup finished\n");
}

unsigned char portspeed_strings[11][11] = {"","FULLSPEED","LOWSPEED","HIGHSPEED","SUPERSPEED"};

void xhci_ring_doorbell(unsigned long slotid,unsigned long value){
	printf("[XHCI] Doorbell: ringing slotid %x with value %x \n",slotid,value);
	doorbell[slotid] = value;
}

struct XHCI_TRB *waitForEvent(unsigned long addr){
	printf("[XHCI] Wait for event %x \n",addr);
	while(1){
		for(int i = 0 ; i < MAX_RING_SIZE ; i++){
			if(event_ring[i].arg1==addr){
				return (struct XHCI_TRB*)&event_ring[i];
			}
		}
	}
}

struct XHCI_TRB *waitForPortAndEndpoint(unsigned char port,unsigned char endpoint,int event_count){
	printf("[XHCI] Wait for port %x and endpoint %x \n",port,endpoint);
	while(1){
		int cx = event_count;
		for(int i = 0 ; i < MAX_RING_SIZE ; i++){
			if(((event_ring[i].arg4>>16) & 0b11111)==endpoint&&((event_ring[i].arg4>>24) & 0b11111111)==port){
				if(cx==0){
					return (struct XHCI_TRB*)&event_ring[i];
				}else{
					cx--;
				}
			}
		}
	}
}

struct XHCI_TRB *setupTRBPackage(struct XHCI_TRB *ring,int queue_index,void *data,unsigned long xferlength, unsigned long flags){
	ring[queue_index].arg1 = (unsigned long)data;
	ring[queue_index].arg2 = 0;
	ring[queue_index].arg3 = xferlength;
	ring[queue_index].arg4 = flags | 1;
	printf("[XHCI] TRB Created: prt=%x %x status=%x control=%x located at=%x \n",ring[queue_index].arg1,ring[queue_index].arg2,ring[queue_index].arg3,ring[queue_index].arg4,(unsigned long)&ring[queue_index]);
	return &ring[queue_index];
}

struct XHCI_TRB *setupCommandRingPackage(void* data,unsigned long xferlength, unsigned long flags){
	struct XHCI_TRB *res = setupTRBPackage((struct XHCI_TRB*)command_ring, ring_queue_index, data, xferlength, flags); // create TRB
	ring_queue_index++; // ring queue index +1
	return res;
}

unsigned char enable_port(){
	struct XHCI_TRB *trb = setupCommandRingPackage(NULL, 0, TRB_TYPE_ENABLE_SLOT<<10); // create required TRB...
	xhci_ring_doorbell(0 ,0); // ring that we are here
	struct XHCI_TRB *result_trb = waitForEvent((unsigned long)trb);
	unsigned char resultcode = (result_trb->arg3>>24)&&0b11111111;
	if(resultcode==1){
		return (result_trb->arg4>>24) & 0b11111111;
	}else{
		printf("[XHCI] Unwanted resultcode: %x \n",resultcode);
		return 0;
	}
}

unsigned char addressdevice(unsigned char device,unsigned long structure){
	struct XHCI_TRB *trb = setupCommandRingPackage(NULL, structure, TRB_TYPE_SET_ADDRESS<<10 | (device << 24)); // create required TRB...
	xhci_ring_doorbell(0 ,0); // ring that we are here
	struct XHCI_TRB *result_trb = waitForEvent((unsigned long)trb);
	unsigned char resultcode = (result_trb->arg3>>24)&&0b11111111;
	return resultcode;
}

unsigned char sendNOOPCommand(struct XHCI_RingManager* ringstat,unsigned char port){
	setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,NULL, 0, (8<<10) ); // create required TRB...
	xhci_ring_doorbell(port ,1); // ring that we are here
	struct XHCI_TRB *pingres = waitForPortAndEndpoint(port, 1, ringstat->event_count);
	ringstat->pointer++;
	ringstat->event_count++;
	return (pingres->arg3>>24)&&0b11111111;
}

unsigned char* xhci_get_device_configuration(struct XHCI_RingManager* ringstat,unsigned char port,unsigned char size){
	void* buffer = malloc(size);
	EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);
	commando->bRequest = 0x06; // get_descriptor
    commando->bRequestType |= 0x80; // dir=IN
    commando->wIndex = 0; // windex=0
    commando->wLength = size; // getlength=8
    commando->wValue = 2 << 8; // get config info
	unsigned long *cmx = (unsigned long*) commando;

	struct XHCI_TRB *tx = setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,commando, 8, (2<<10) | (1<<6) | (3<<16) ); // SETUP
	tx->arg1 = cmx[0];
	tx->arg2 = cmx[1];
	ringstat->pointer++;
	setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,buffer, size, (3<<10) | (1<<16) ); // DATA
	ringstat->pointer++;
	setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,NULL, 0, (4<<10) | (0<<16) ); // STATUS
	ringstat->pointer++;
	
	xhci_ring_doorbell(port, 1); // ring that we are here
	struct XHCI_TRB *pingres = waitForPortAndEndpoint(port, 1, ringstat->event_count);
	ringstat->event_count++;

	// cleanup crapas
	free(commando);
	if(((pingres->arg3>>24)&&0b11111111)==1){
		return buffer;
	}else{
		return NULL;
	}
}

unsigned char* xhci_get_device_descriptor(struct XHCI_RingManager* ringstat,unsigned char port,unsigned char size){
	void* buffer = malloc(size);
	EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);
	commando->bRequest = 0x06; // get_descriptor
    commando->bRequestType |= 0x80; // dir=IN
    commando->wIndex = 0; // windex=0
    commando->wLength = size; // getlength=8
    commando->wValue = 1 << 8; // get device info

	setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,commando, 8, (2<<10) | (1<<6) | (3<<16) ); // SETUP
	ringstat->pointer++;
	setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,buffer, size, (3<<10) | (1<<16) ); // DATA
	ringstat->pointer++;
	setupTRBPackage((struct XHCI_TRB*)ringstat->ring,ringstat->pointer,NULL, 0, (4<<10) | (1<<5) | (0<<16) ); // STATUS
	ringstat->pointer++;
	
	xhci_ring_doorbell(port ,1); // ring that we are here
	struct XHCI_TRB *pingres = waitForPortAndEndpoint(port, 1, ringstat->event_count);
	ringstat->event_count++;

	// cleanup crapas
	free(commando);
	if(((pingres->arg3>>24)&&0b11111111)==1){
		return buffer;
	}else{
		return NULL;
	}
}

void xhci_setup_port(volatile struct XHCI_OperationalPortRegisters portreg,unsigned char portnumber){
	if(portreg.PORTSC==0){
		return;
	}
	portnumber++;
	// printf("[XHCI] Portscan : port %x has value %x state %x \n",portnumber,portreg.PORTSC,(portreg.PORTSC>>5)&0b111);
	if(portreg.PORTSC & (1<<17)){ // is there a connect chance?
		operational->USBSTS |= 0x18; // acknowledge interrupts!
		portreg.PORTSC |= (1<<17); // acknowledge chance!
		if(portreg.PORTSC & 1){
			printf("[XHCI] Port %x: There is a device present %x \n",portnumber,portreg.PORTSC);
			// lets find out the portspeed
			unsigned char portspeed_probe = (portreg.PORTSC >> 10) & 0b111;
			unsigned char portstat = (portreg.PORTSC>>5)&0b111;
			if(portstat==7){
				printf("[XHCI] Port %x: This is a USB2.0 port, Requires portreset\n");
				operational->USBSTS |= 0x10;
				operational_port[portnumber-1].PORTSC |= (1<<4);
				while((operational->USBSTS & 0x10)==0);
				portreg = operational_port[portnumber-1];
				portspeed_probe = (portreg.PORTSC >> 10) & 0b111;
				portstat = (portreg.PORTSC>>5)&0b111;
				operational->USBSTS |= 0x10;
			}
			printf("[XHCI] Port %x: This is a %s port with the status of %x and the rest is %x !\n",portnumber,portspeed_strings[portspeed_probe],portstat,portreg.PORTSC);

			//
			// enable port
			unsigned char enable_port_result = enable_port();
			if(!enable_port_result){
				return;
			}
			printf("[XHCI] Port %x: We have the following portnumber assigned: %x \n",portnumber,enable_port_result);

			//
			// create structures for next steps
			// input context data structure
			// input control context
			struct XHCI_InputControlContext* icc = (struct XHCI_InputControlContext*) malloc_align(sizeof(struct XHCI_InputControlContext) * 4 ,0xFF);
			icc->A = 0b11;
			// input slot context
			struct XHCI_Context* isc = (struct XHCI_Context*) (icc + sizeof(struct XHCI_Context));
			isc->arg1 |= (1<<27); // context entries: 1
			isc->arg1 |= (3<<20); // speed: 3 highspeed
			isc->arg2 |= (portnumber<<16);
			isc->arg3 |= enable_port_result; // id of the port
			isc->arg3 |= (portnumber<<8); // portnumber
			// allocate transfer ring
			volatile struct XHCI_TRB *transfer_ring = (volatile struct XHCI_TRB *) malloc_align(WANTED_RING_SIZE,0xFF);
			// input default control endpoint
			struct XHCI_Context* idc = (struct XHCI_Context*) (icc + sizeof(struct XHCI_Context) + sizeof(struct XHCI_Context));
			idc->arg2 |= (4<<3); // set ep type to control
			idc->arg2 |= (3<<1); // set cerr to 3
			idc->arg2 |= (64<<16); // max packet size
			idc->arg3 |= (unsigned long)transfer_ring; // transfer ring
			idc->arg3 |= 1; // dequeue cycle state
			// output device context
			struct XHCI_Context* odc = (struct XHCI_Context*) (icc + sizeof(struct XHCI_Context) + sizeof(struct XHCI_Context) + sizeof(struct XHCI_Context));
			// attach dcbaa
			DCBAAP[enable_port_result].ptr_low = (unsigned long)odc;
			DCBAAP[enable_port_result].ptr_high = 0;
			// address device command
			unsigned char addressdevice_result = addressdevice(enable_port_result,(unsigned long)icc);
			if(addressdevice_result!=1){
				printf("[XHCI] Port %x: set_address failed with %x \n",portnumber,addressdevice_result);
				return;
			}
			printf("[XHCI] Port %x: set_address succeed\n",portnumber);

			struct XHCI_RingManager* ringstat = (struct XHCI_RingManager*) malloc(sizeof(struct XHCI_RingManager));
			ringstat->cs = 1;
			ringstat->pointer = 0;
			ringstat->event_count = 0;
			ringstat->ring = transfer_ring;

			//
			// check with a NOOP command
			unsigned char pingrescod = sendNOOPCommand(ringstat,enable_port_result);
			if(pingrescod!=1){
				printf("[XHCI] Port %x: noopcmd failed with %x \n",portnumber,pingrescod);
				return;
			}
			printf("[XHCI] Port %x: get_noop succeed \n",portnumber);

			// unsigned char* desc = xhci_get_device_descriptor(ringstat,enable_port_result,8);
			// if(!desc){
			// 	printf("[XHCI] Port %x: xhci_get_device_descriptor failed with %x \n",portnumber,desc);
			// 	return;
			// }
			// printf("[XHCI] Port %x: xhci_get_device_descriptor succeed \n",portnumber);
			// if(!(desc[0]==0x12&&desc[1]==0x1)){
			// 	printf("[XHCI] Port %x: Invalid device descriptor! \n");
			// 	for(;;);
			// }
			// unsigned char maxpacketsize = desc[7];
			// printf("[XHCI] Port %x: maxpacketsize=%x \n",portnumber,maxpacketsize);
			// for(;;);

			usb_config_descriptor* sec = (usb_config_descriptor*)xhci_get_device_configuration(ringstat,enable_port_result,sizeof(usb_interface_descriptor) + sizeof(usb_config_descriptor)+(sizeof(EHCI_DEVICE_ENDPOINT)*2));
			if(!sec){
				printf("[XHCI] Port %x: xhci_get_device_configuration failed with %x \n",portnumber,sec);
				return;
			}
			printf("[XHCI] Port %x: xhci_get_device_configuration succeed \n",portnumber);
        	usb_interface_descriptor* desc2 = (usb_interface_descriptor*)(((unsigned long)sec)+sizeof(usb_config_descriptor));
			printf("[XHCI] Port %x: deviceclass is %x \n",portnumber,desc2->bInterfaceClass);
			for(;;);

			USB_DEVICE *device = (USB_DEVICE*) malloc(sizeof(USB_DEVICE));
			device->drivertype = 3;

			usb_device_install(device);

			for(;;);
		}
	}
}

void xhci_detect_ports(){
	for(unsigned char i = 0 ; i < ((capability->HCCPARAMS1 >> 24) & 0xFF) ; i++){
		volatile struct XHCI_OperationalPortRegisters x = operational_port[ i ];
		xhci_setup_port(x,i);
	}
}

void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	unsigned long bar1 = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFF0;
	unsigned long bar2 = getBARaddress(bus,slot,function,0x14);

	if(!pci_enable_busmastering_when_needed(bus,slot,function)){
        // return;
    }

	//
	// display version info
	printf("[XHCI] XHCI driver started at address %x %x \n",bar1,bar2);
	if(bar2){
		printf("[XHCI] XHCI driver is in 64bit mode while we are 32bit\n");
		return;
	}
	unsigned char sbrn = getBARaddress(bus,slot,function,0x60) & 0xFF;
	printf("[XHCI] SBRN=%x \n",sbrn);

	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)xhciirq);

	//
	// what is our default state?
	capability = (struct XHCI_CapibilityRegisters*) bar1;
	xhci_dump_capibility(capability);
	operational = (volatile struct XHCI_OperationalRegisters *) (bar1 + capability->CAPLENGTH);
	operational_port = (volatile struct XHCI_OperationalPortRegisters *) (bar1 + capability->CAPLENGTH + 0x400 );
	runtime = (volatile struct XHCI_RuntimeRegisters *) (bar1 + (capability->RTSOFF&0xFFFFFFE0) );
	doorbell = (unsigned long*) (bar1 + capability->DBOFF);
	xhci_dump_operational(operational);
	printf("[XHCI] We have %x ports available to us!\n",(capability->HCCPARAMS1 >> 24) & 0xFF );

	//
	// detecting extended capabilities
	if(capability->HCCPARAMS1 & 0xFFFF0000){
		unsigned long extcappoint = bar1 + ((capability->HCCPARAMS1 &0xFFFF0000)>>14);
		printf("[XHCI] We have capabilities: %x \n",extcappoint);
		for(int i = 0 ; i < 20 ; i++){
			volatile unsigned long fault = ((volatile unsigned long*)(extcappoint))[0];
			unsigned char capid = fault & 0xFF;
			unsigned char capof = (fault >> 8) & 0xFF;
			if(capid==0||capof==0){
				break;
			}
			if(capid==0x01){
				printf("[XHCI] USB Legacy Support\n");
				if(fault&(1<<16)){
					printf("[XHCI] BIOS owns our controller!\n");
					((volatile unsigned long*)(extcappoint))[0] |= (1<<24);
					while(((volatile unsigned long*)(extcappoint))[0]&(1<<16));
					printf("[XHCI] Lets see if we are the ownernow...\n");
					fault = ((volatile unsigned long*)(extcappoint))[0];
					if(fault&(1<<16)){
						printf("[XHCI] Sadly we failed\n");
					}else{
						printf("[XHCI] We own the controller now!\n");
					}
				}
			}
			else if(capid==0x02){
				printf("[XHCI] Supported Protocol\n");
				unsigned char revision_minor = (fault>>16) & 0xFF;
				unsigned char revision_major = (fault>>24) & 0xFF;
				unsigned long fault2 = ((volatile unsigned long*)(extcappoint+8))[0];
				unsigned char comoffset = (fault2) & 0xFF;
				unsigned char comcount = (fault2>>8) & 0xFF;
				printf("[XHCI] Revision %x %x , offset %x , count %x \n",revision_major,revision_minor,comoffset,comcount);
			}
			else if(capid==0x03){
				printf("[XHCI] Extended Power Management\n");
			}
			else if(capid==0x04){
				printf("[XHCI] I/O Virtualization\n");
			}
			else if(capid==0x05){
				printf("[XHCI] PCI MSI Configuration Capability Structure\n");
			}
			else if(capid==0x06){
				printf("[XHCI] Local Memory\n");
			}
			else if(capid==0x0A){
				printf("[XHCI] USB Debug Capability\n");
			}
			else if(capid==0x11){
				printf("[XHCI] MSI-X Configuration Capability Structure\n");
			}
			extcappoint += (capof*sizeof(unsigned long));
		}
	}

	//
	// reset
	xhci_reset();
	if(operational->USBSTS&4){ // serious error bit set
		printf("[XHCI] An serious error occured\n");
		for(;;);
	}

	//
	// detect devices
	xhci_detect_ports();

	//
	// This message should never ever occur
	if(operational->USBSTS&4){ // serious error bit set
		printf("[XHCI] An serious error occured\n");
		for(;;);
	}
	
	printf("[XHCI] Hang....\n");
	for(;;);
}
