#include "../kernel.h"

#define XHCI_BAR_MASK 0xFFFFFFF0

#define XHCI_HCSPARAMS1_MAXSLOTS(x) ( x & 0xFF )
#define XHCI_HCSPARAMS1_MAXINTRS(x) ( (x>>8) & 0x7FF )
#define XHCI_HCSPARAMS1_MAXPORTS(x) ( (x>>24) & 0xFF )

#define XHCI_HCSPARAMS2_IST(x) ( (x&0b111) & 0xF )
#define XHCI_HCSPARAMS2_ERSTMAX(x) ( (x>>4) & 0b1111 )
#define XHCI_HCSPARAMS2_MSBHI(x) ( (x>>21) & 0b11111 )
#define XHCI_HCSPARAMS2_SPR(x) ( (x>>26) & 0b1 )
#define XHCI_HCSPARAMS2_MSBLO(x) ( (x>>27) & 0b11111 )

#define XHCI_HCCPARAMS1_XECP(x) ((x>>16)&0b1111111111111111)

struct XHCIHostControllerCapabilityRegisters{
	unsigned char CAPLENGTH; // Capability Register Length
	unsigned char res; // reserved
	unsigned short HCIVERSION; // Interface Version Number
	unsigned long HCSPARAMS1; // Structural Parameters 1
	unsigned long HCSPARAMS2; // Structural Parameters 2
	unsigned long HCSPARAMS3; // Structural Parameters 3
	unsigned long HCCPARAMS1; // Capability Parameters 1
	unsigned long DBOFF; // Doorbell Offset
	unsigned long RTSOFF; // Runtime Register Space Offset
	unsigned long HCCPARAMS2; // Capability Parameters 2
} __attribute__((packed));

unsigned long BAR1;
unsigned long extended_cap_addr;
volatile struct XHCIHostControllerCapabilityRegisters *cap_reg;
unsigned long ope_reg;
unsigned long *bcbaap;
unsigned long *commandring;
unsigned long *eventring;

void xhci_dump_capability_registers(){
	printf("[XHCI] caplength   : %x \n",cap_reg->CAPLENGTH);
	printf("[XHCI] hciversion  : %x \n",cap_reg->HCIVERSION);
	printf("[XHCI] hcsparams1  : %x \n",cap_reg->HCSPARAMS1);
	printf("[XHCI]   -MaxSlots : %x \n",XHCI_HCSPARAMS1_MAXSLOTS(cap_reg->HCSPARAMS1));
	printf("[XHCI]   -MaxIntrs : %x \n",XHCI_HCSPARAMS1_MAXINTRS(cap_reg->HCSPARAMS1));
	printf("[XHCI]   -MaxPorts : %x \n",XHCI_HCSPARAMS1_MAXPORTS(cap_reg->HCSPARAMS1));
	printf("[XHCI] hcsparams2  : %x \n",cap_reg->HCSPARAMS2);
	printf("[XHCI]   -IST      : %x \n",XHCI_HCSPARAMS2_IST(cap_reg->HCSPARAMS2));
	printf("[XHCI]   -ERSTMax  : %x \n",XHCI_HCSPARAMS2_ERSTMAX(cap_reg->HCSPARAMS2));
	printf("[XHCI]   -MSBHi    : %x \n",XHCI_HCSPARAMS2_MSBHI(cap_reg->HCSPARAMS2));
	printf("[XHCI]   -SPR      : %x \n",XHCI_HCSPARAMS2_SPR(cap_reg->HCSPARAMS2));
	printf("[XHCI]   -MSBLo    : %x \n",XHCI_HCSPARAMS2_MSBLO(cap_reg->HCSPARAMS2));
	printf("[XHCI] hcsparams3  : %x \n",cap_reg->HCSPARAMS3);
	printf("[XHCI] hccparams1  : %x \n",cap_reg->HCCPARAMS1);
	printf("[XHCI]   -XECP     : %x \n",XHCI_HCCPARAMS1_XECP(cap_reg->HCCPARAMS1));
	printf("[XHCI] dboff       : %x \n",cap_reg->DBOFF);
	printf("[XHCI] rtsoff      : %x \n",cap_reg->RTSOFF);
	printf("[XHCI] hccparams2  : %x \n",cap_reg->HCCPARAMS2);
}

unsigned long get_xhci_operational_usbcmd_addr(){
	return ope_reg + 0;
}

unsigned long get_xhci_operational_usbcmd(){
	return ((unsigned long*)get_xhci_operational_usbcmd_addr())[0];
}

void set_xhci_operational_usbcmd(unsigned long val){
	((unsigned long*)get_xhci_operational_usbcmd_addr())[0] = val;
}

unsigned long get_xhci_operational_usbsts_addr(){
	return ope_reg + 4;
}

unsigned long get_xhci_operational_usbsts(){
	return ((unsigned long*)get_xhci_operational_usbsts_addr())[0];
}

void set_xhci_operational_usbsts(unsigned long val){
	((unsigned long*)get_xhci_operational_usbsts_addr())[0] = val;
}

unsigned long get_xhci_operational_usbconfig_addr(){
	return ope_reg + 0x38;
}

unsigned long get_xhci_operational_usbconfig(){
	return ((unsigned long*)get_xhci_operational_usbconfig_addr())[0];
}

void set_xhci_operational_usbconfig(unsigned long val){
	((unsigned long*)get_xhci_operational_usbconfig_addr())[0] = val;
}

unsigned long get_xhci_operational_usbbcbaap_addr(){
	return ope_reg + 0x30;
}

unsigned long get_xhci_operational_usbbcbaap(){
	return ((unsigned long*)get_xhci_operational_usbbcbaap_addr())[0];
}

void set_xhci_operational_usbbcbaap(unsigned long val){
	((unsigned long*)get_xhci_operational_usbbcbaap_addr())[0] = val;
	((unsigned long*)get_xhci_operational_usbbcbaap_addr())[1] = 0;
}

unsigned long get_xhci_operational_usbcrcr_addr(){
	return ope_reg + 0x18;
}

unsigned long get_xhci_operational_usbcrcr(){
	return ((unsigned long*)get_xhci_operational_usbcrcr_addr())[0];
}

void set_xhci_operational_usbcrcr(unsigned long val){
	((unsigned long*)get_xhci_operational_usbcrcr_addr())[0] = val;
	((unsigned long*)get_xhci_operational_usbcrcr_addr())[1] = 0;
}

void wait_untill_xhci_is_ready(){
	while(get_xhci_operational_usbsts() & 0x800);
}

void xhci_send_reset_controller_command(){
	printf("[XCHI] Triggering XHCI reset...\n");
	set_xhci_operational_usbcmd( get_xhci_operational_usbcmd() | 2 );
	wait_untill_xhci_is_ready();
	printf("[XCHI] Finished XHCI reset!\n");
}

void xhci_set_max_used_port_to_config(){
	printf("[XCHI] Tell XHCI we want to have %x ports!\n",XHCI_HCSPARAMS1_MAXPORTS(cap_reg->HCSPARAMS1));
	set_xhci_operational_usbconfig(get_xhci_operational_usbconfig() & XHCI_HCSPARAMS1_MAXPORTS(cap_reg->HCSPARAMS1) );
}

unsigned long get_xhci_operational_usberstsz_addr(int intno){
	return BAR1 + cap_reg->RTSOFF + (0x20 * (intno + 1) ) + 8;
}

unsigned long get_xhci_operational_usberstsz(int intno){
	return ((unsigned long*)get_xhci_operational_usberstsz_addr(intno))[0];
}

void set_xhci_operational_usberstsz(int intno,unsigned long val){
	((unsigned long*)get_xhci_operational_usberstsz_addr(intno))[0] = val;
}

unsigned long get_xhci_operational_usberdp_addr(int intno){
	return BAR1 + cap_reg->RTSOFF + (0x20 * (intno + 1) ) + 0x18;
}

unsigned long get_xhci_operational_usberdp(int intno){
	return ((unsigned long*)get_xhci_operational_usberdp_addr(intno))[0];
}

void set_xhci_operational_usberdp(int intno,unsigned long val){
	((unsigned long*)get_xhci_operational_usberdp_addr(intno))[0] = val;
	((unsigned long*)get_xhci_operational_usberdp_addr(intno))[1] = 0;
}

unsigned long get_xhci_operational_usberstba_addr(int intno){
	return BAR1 + cap_reg->RTSOFF + (0x20 * (intno + 1) ) + 0x18;
}

unsigned long get_xhci_operational_usberstba(int intno){
	return ((unsigned long*)get_xhci_operational_usberstba_addr(intno))[0];
}

void set_xhci_operational_usberstba(int intno,unsigned long val){
	((unsigned long*)get_xhci_operational_usberstba_addr(intno))[0] = val;
	((unsigned long*)get_xhci_operational_usberstba_addr(intno))[1] = 0;
}

unsigned long get_xhci_operational_usbportsc_addr(int port){
	return ope_reg + 0x400 + (0x10 * port);
}

unsigned long get_xhci_operational_usbportsc(int port){
	return ((unsigned long*)get_xhci_operational_usbportsc_addr(port))[0];
}

void set_xhci_operational_usbportsc(int port, unsigned long val){
	((unsigned long*)get_xhci_operational_usbportsc_addr(port))[0] = val;
}

void xhci_check_port(int portindex){

	// is there a chance?
	if(!(get_xhci_operational_usbportsc(portindex)&0x20000)){
		return;
	}

	// acknowledge changes...
	set_xhci_operational_usbportsc(portindex, get_xhci_operational_usbportsc(portindex));

	if(!(get_xhci_operational_usbportsc(portindex)&1)){
		printf("[XHCI] Port %x : slot is empty \n",portindex);
		return;
	}

	if(get_xhci_operational_usbportsc(portindex)&2){
		printf("[XHCI] Port %x : USB3.0 \n",portindex);
	}else if(get_xhci_operational_usbportsc(portindex)&1){
		printf("[XHCI] Port %x : USB2.0 \n",portindex);
		unsigned long a = get_xhci_operational_usbportsc(portindex);
		a |= 0b1000010000;
		set_xhci_operational_usbportsc(portindex, a );
		sleep(100);
		set_xhci_operational_usbportsc(portindex, get_xhci_operational_usbportsc(portindex));
	}
	printf("[XHCI] Port %x : port is present here with status: %x \n",portindex,get_xhci_operational_usbportsc(portindex));
}

void probe_xhci(){
	for(int i = 0 ; i < XHCI_HCSPARAMS1_MAXPORTS(cap_reg->HCSPARAMS1) ; i++){
		xhci_check_port(i);
	}
}

/**
 * Installs XHCI. This is the main function
 * intell uses xhci offset 0x47c
 * \param bus bus to check it
 * \param slot slot to check 
 * \param function function to check
 */
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function){
	printf("[XHCI] entering xhci driver....\n");
	printf("[XHCI] bus=%x slot=%x function=%x \n",bus,slot,function);
	
	BAR1 = getBARaddress(bus,slot,function,0x10) & XHCI_BAR_MASK;
	printf("[XHCI] Base address 1=%x \n",BAR1);

	cap_reg = (volatile struct XHCIHostControllerCapabilityRegisters*) BAR1;
	ope_reg =  BAR1 + cap_reg->CAPLENGTH;
	xhci_dump_capability_registers();

	// time to parse the info!
	extended_cap_addr = BAR1 + (XHCI_HCCPARAMS1_XECP(cap_reg->HCCPARAMS1) << 2);
	unsigned long extended_cap_addr_offset = extended_cap_addr;
	while(1){
		unsigned long first_extended_cap = ((unsigned long*)extended_cap_addr_offset)[0];
		unsigned char cap_id = first_extended_cap & 0xFF;
		unsigned char cap_space = (first_extended_cap>>8) & 0xFF;
		if(cap_id==1){
			printf("[XHCI] Found USB Legacy Support capability\n");
			if(first_extended_cap&0x1010000){
				printf("[XHCI] Owner is specified!\n");
				if(first_extended_cap&0x10000){
					printf("[XHCI] Currently, BIOS owns XHCI controller\n");
					((unsigned long*)extended_cap_addr_offset)[0] |= 0x1000000;
					while(((unsigned long*)extended_cap_addr_offset)[0]&0x10000);
				}
				if(((unsigned long*)extended_cap_addr_offset)[0]&0x1000000){
					printf("[XHCI] Currently, We own the XHCI controller\n");
				}
			}
		}else if(cap_id==2){
			printf("[XHCI] Found Supported Protocol capability\n");
		}else if(cap_id==3){
			printf("[XHCI] Found Extended Power Management capability\n");
		}else if(cap_id==4){
			printf("[XHCI] Found I/O Virtualization capability\n");
		}else if(cap_id==5){
			printf("[XHCI] Found Message Interrupt capability\n");
		}else if(cap_id==6){
			printf("[XHCI] Found Local Memory capability\n");
		}else if(cap_id==10){
			printf("[XHCI] Found USB Debug Capability capability\n");
		}else if(cap_id==17){
			printf("[XHCI] Found Extended Message Interrupt capability\n");
		}else if(cap_id>=0xC0){
			printf("[XHCI] Found Vendor Defined capability\n");
		}else{
			printf("[XHCI] Found unknown extended capability: %x \n",cap_id);
		}
		extended_cap_addr_offset += (cap_space*sizeof(unsigned long));
		if(cap_space==0){
			break;
		}
	}

	// trigger reset!
	xhci_send_reset_controller_command();

	// tell what our max port is
	xhci_set_max_used_port_to_config();

	// ERSTSZ
	set_xhci_operational_usberstsz(0,get_xhci_operational_usberstsz(1) & 1);

	// ERDP
	eventring = (unsigned long*) malloc(sizeof(unsigned long)*20);
	set_xhci_operational_usberdp(0, (unsigned long) eventring);

	// ERSTBA
	unsigned long rsb1[20]  __attribute__ ((aligned (0x100)));
	unsigned long rsb2 = ((unsigned long)&rsb1)+4;
	unsigned long rsb3 = ((unsigned long)&rsb1)+8;
	
	((unsigned long*)&rsb1)[0] = ((unsigned long)commandring);
	((unsigned long*)rsb2)[0] = 0;
	((unsigned long*)rsb3)[0] |= 16;
	set_xhci_operational_usberstba(0,(unsigned long)&rsb1);

	// setup crcr
	commandring = (unsigned long*) malloc_align(sizeof(unsigned long)*20,0xFFF);
	printf("[XHCI] CRCR is set to : %x \n",commandring);
	set_xhci_operational_usbcrcr(((unsigned long)commandring) | 1);

	// setup bcbaap register
	bcbaap = (unsigned long*) malloc_align(sizeof(unsigned long)*20,0xFFF);
	printf("[XHCI] BCBAAP is set to : %x \n",bcbaap);
	set_xhci_operational_usbbcbaap((unsigned long) bcbaap);

	// lets go!
	set_xhci_operational_usbcmd( get_xhci_operational_usbcmd() | 1 );
	printf("[XHCI] is running now!\n");

	while(1){
		sleep(200);
		probe_xhci();
	}
	
	for(;;);
}

unsigned char* xhci_send_and_recieve_command(USB_DEVICE *device,EhciCMD* commando,void *buffer){}

void irq_xhci(){}

unsigned long xhci_send_bulk(USB_DEVICE *device,unsigned char* out,unsigned long expectedOut){}