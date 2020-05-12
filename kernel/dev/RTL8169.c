#include "../kernel.h"

//
// FROM: https://wiki.osdev.org/RTL8169
struct Descriptor{
	unsigned int command;  /* command/status uint32_t */
	unsigned int vlan;     /* currently unused */
	unsigned int low_buf;  /* low 32-bits of physical buffer address */
	unsigned int high_buf; /* high 32-bits of physical buffer address */
};

struct PackageRecievedDescriptor{
	unsigned long buffersize;
	unsigned long low_buf;
	unsigned long high_buf;
};

/** 
* Note that this assumes 16*1024=16KB (4 pages) of physical memory at 1MB and 2MB is identity mapped to 
* the same linear address range
*/
struct Descriptor *Rx_Descriptors = (struct Descriptor *)0x1000; /* 1MB Base Address of Rx Descriptors */
struct Descriptor *Tx_Descriptors = (struct Descriptor *)0x2000; /* 2MB Base Address of Tx Descriptors */

unsigned long num_of_rx_descriptors = 1024;
unsigned long num_of_tx_descriptors = 1024;
unsigned long bar1 = 0;
unsigned long rx_pointer = 0;
unsigned volatile long package_recieved_ack = 0;

extern void rtl8169irq();

void irq_rtl8169(){
	printf("[RTL81] Interrupt detected\n");
	unsigned short status = inportw(bar1 + 0x3E);
	if(status&0x20){
		printf("[RTL81] Link change detected!\n");
		status |= 0x20;
	}
	if(status&0x01){
		printf("[RTL81] Package recieved!\n");
		((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0] = 1;
		status |= 0x01;
	}
	outportw(bar1 + 0x3E,status);
	
	status = inportw(bar1 + 0x3E);
	if(status!=0x00){
		printf("[RTL81] Unresolved interrupt: %x \n",status);
	}
	
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

struct PackageRecievedDescriptor rtl_recievePackage(){
	((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0] = 0;
	while(1){
		unsigned volatile long x = ((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0];
		if(x==1){
			break;
		}
	}
	struct Descriptor desc = Rx_Descriptors[rx_pointer++];
	struct PackageRecievedDescriptor res;
	unsigned long buffer_size = desc.command & 0x3FFF;
	res.buffersize = buffer_size;
	res.low_buf = desc.low_buf;
	res.high_buf = desc.high_buf;
	return res;
}

void init_rtl(int bus,int slot,int function){
	printf("[RTL81] Driver loaded\n");
	bar1 = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFFE;
	printf("[RTL81] BAR=%x \n",bar1);
	
	printf("[RTL81] Set interrupter\n");
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)rtl8169irq);
	
	//
	// trigger reset
	printf("[RTL81] Resetting driver \n");
	outportb(bar1 + 0x37, 0x10);
	while(inportb(bar1 + 0x37) & 0x10){}
	printf("[RTL81] Driver reset succesfully \n");
	
	//
	// get mac address
	// testcomputer: 1C 6F 65 70 F2 FC
	unsigned char macaddress[6];
	for(int i = 0 ; i < 6 ; i++){
		macaddress[i] = inportb(bar1+i);
	}
	printf("[RTL81] MAC-address %x %x %x %x %x %x \n",macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);
	
	//
	// setup rx descriptor
	printf("[RTL81] Setup RX descriptor\n");
	unsigned int OWN = 0x80000000;
	unsigned int EOR = 0x40000000;
	for(unsigned long i = 0; i < num_of_rx_descriptors; i++){
		unsigned long rx_buffer_len = 100;
		unsigned long packet_buffer_address = (unsigned long)malloc(rx_buffer_len);
		if(i == (num_of_rx_descriptors - 1)){
			Rx_Descriptors[i].command = (OWN | EOR | (rx_buffer_len & 0x3FFF));
		}else{
			Rx_Descriptors[i].command = (OWN | (rx_buffer_len & 0x3FFF));
		}
		Rx_Descriptors[i].low_buf = (unsigned int)packet_buffer_address;
		Rx_Descriptors[i].high_buf = 0;
	}
	
	outportb(bar1 + 0x50, 0xC0); /* Unlock config registers */
	outportl(bar1 + 0x44, 0x0000E70F); /* RxConfig = RXFTH: unlimited, MXDMA: unlimited, AAP: set (promisc. mode set) */
	outportb(bar1 + 0x37, 0x04); /* Enable Tx in the Command register, required before setting TxConfig */
	outportl(bar1 + 0x40, 0x03000700); /* TxConfig = IFG: normal, MXDMA: unlimited */
	outportw(bar1 + 0xDA, 0x1FFF); /* Max rx packet size */
	outportb(bar1 + 0xEC, 0x3B); /* max tx packet size */
	outportl(bar1 + 0x20, (unsigned long)&Tx_Descriptors[0]); /* Tell the NIC where the first Tx descriptor is */
	outportl(bar1 + 0xE4, (unsigned long)&Rx_Descriptors[0]); /* Tell the NIC where the first Rx descriptor is */
	outportw(bar1 + 0x3C, 0xC1FF); /* Eigen initiatief: Interrupt mask */
	outportb(bar1 + 0x37, 0x0C); /* Enable Rx/Tx in the Command register */
	outportb(bar1 + 0x50, 0x00); /* Lock config registers */
	
	printf("[RTL81] Setup finished\n");
	struct PackageRecievedDescriptor res = rtl_recievePackage();
	printf("[RTL81] Testpackage recieved. Length=%x \n",res.buffersize);
	for(unsigned long i = 0 ; i < res.buffersize ; i++){
		unsigned char val = ((unsigned char*)res.low_buf)[i];
		printf("%x ",val);
	}
	for(;;);
}
