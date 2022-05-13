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
struct XHCIHostControllerCapabilityRegisters *cap_reg;

void xhci_dump_capability_registers(struct XHCIHostControllerCapabilityRegisters* regs){
	printf("[XHCI] caplength   : %x \n",regs->CAPLENGTH);
	printf("[XHCI] hciversion  : %x \n",regs->HCIVERSION);
	printf("[XHCI] hcsparams1  : %x \n",regs->HCSPARAMS1);
	printf("[XHCI]   -MaxSlots : %x \n",XHCI_HCSPARAMS1_MAXSLOTS(regs->HCSPARAMS1));
	printf("[XHCI]   -MaxIntrs : %x \n",XHCI_HCSPARAMS1_MAXINTRS(regs->HCSPARAMS1));
	printf("[XHCI]   -MaxPorts : %x \n",XHCI_HCSPARAMS1_MAXPORTS(regs->HCSPARAMS1));
	printf("[XHCI] hcsparams2  : %x \n",regs->HCSPARAMS2);
	printf("[XHCI]   -IST      : %x \n",XHCI_HCSPARAMS2_IST(regs->HCSPARAMS2));
	printf("[XHCI]   -ERSTMax  : %x \n",XHCI_HCSPARAMS2_ERSTMAX(regs->HCSPARAMS2));
	printf("[XHCI]   -MSBHi    : %x \n",XHCI_HCSPARAMS2_MSBHI(regs->HCSPARAMS2));
	printf("[XHCI]   -SPR      : %x \n",XHCI_HCSPARAMS2_SPR(regs->HCSPARAMS2));
	printf("[XHCI]   -MSBLo    : %x \n",XHCI_HCSPARAMS2_MSBLO(regs->HCSPARAMS2));
	printf("[XHCI] hcsparams3  : %x \n",regs->HCSPARAMS3);
	printf("[XHCI] hccparams1  : %x \n",regs->HCCPARAMS1);
	printf("[XHCI]   -XECP     : %x \n",XHCI_HCCPARAMS1_XECP(regs->HCCPARAMS1));
	printf("[XHCI] dboff       : %x \n",regs->DBOFF);
	printf("[XHCI] rtsoff      : %x \n",regs->RTSOFF);
	printf("[XHCI] hccparams2  : %x \n",regs->HCCPARAMS2);
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

	cap_reg = (struct XHCIHostControllerCapabilityRegisters*) BAR1;
	xhci_dump_capability_registers(cap_reg);

	for(;;);
}

unsigned char* xhci_send_and_recieve_command(USB_DEVICE *device,EhciCMD* commando,void *buffer){}

void irq_xhci(){}

unsigned long xhci_send_bulk(USB_DEVICE *device,unsigned char* out,unsigned long expectedOut){}