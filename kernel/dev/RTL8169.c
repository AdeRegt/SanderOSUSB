//
// includes.... everything in one file for simplicity
#include "../kernel.h"

//
// How to run networking on qemu:
// See main-tread: https://forum.osdev.org/viewtopic.php?f=1&t=30546
// Requires: sudo apt-get install bridge-utils
// sudo brctl addbr br0
// sudo ../../qemu/x86_64-softmmu/qemu-system-x86_64 -netdev tap,id=thor_net0 -device rtl8139,netdev=thor_net0,id=thor_nic0 -kernel ../kernel.bin
//

//
// FROM: https://wiki.osdev.org/RTL8169
// This is a entry in the queue of the recieve and transmit descriptor queue
struct Descriptor{
	unsigned int command;  /* command/status uint32_t */
	unsigned int vlan;     /* currently unused */
	unsigned int low_buf;  /* low 32-bits of physical buffer address */
	unsigned int high_buf; /* high 32-bits of physical buffer address */
};

// The recieve and transmit queue
struct Descriptor *Rx_Descriptors; /* 1MB Base Address of Rx Descriptors */
struct Descriptor *Tx_Descriptors; /* 2MB Base Address of Tx Descriptors */

// For loops
unsigned long num_of_rx_descriptors = 1024;
unsigned long num_of_tx_descriptors = 1024;
unsigned long bar1 = 0;
unsigned long rx_pointer = 0;
unsigned long tx_pointer = 0;
unsigned volatile long package_recieved_ack = 0;
unsigned volatile long package_send_ack = 0;

extern void rtl8169irq();

void irq_rtl8169(){
	debugf("[RTL81] Interrupt detected\n");
	unsigned short status = inportw(bar1 + 0x3E);
	if(status&0x20){
		debugf("[RTL81] Link change detected!\n");
		ethernet_set_link_status(1);
		status |= 0x20;
	}
	if(status&0x01){
		debugf("[RTL81] Package recieved!\n");
		((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0] = 1;
		status |= 0x01;
	}
	if(status&0x04){
		debugf("[RTL81] Package send!\n");
		((unsigned volatile long*)((unsigned volatile long)&package_send_ack))[0] = 1;
		status |= 0x04;
	}
	outportw(bar1 + 0x3E,status);
	
	status = inportw(bar1 + 0x3E);
	if(status!=0x00){
		debugf("[RTL81] Unresolved interrupt: %x \n",status);
	}
	
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

void rtl_sendPackage(PackageRecievedDescriptor desc,unsigned char first,unsigned char last,unsigned char ip,unsigned char udp, unsigned char tcp){
	unsigned long ms1 = 0x80000000 | 0x40000000 | (ip==1?0x40000:0) | (udp==1?0x20000:0) | (tcp==1?0x10000:0) | (first==1?0x20000000:0) | (last==1?0x10000000:0) | (desc.buffersize & 0x3FFF); // 0x80000000 | ownbit=yes | firstsegment | lastsegment | length
	unsigned long ms2 = 0 ;
	unsigned long ms3 = desc.low_buf;
	unsigned long ms4 = desc.high_buf;
	
	volatile struct Descriptor *desz = ((volatile struct Descriptor*)(Tx_Descriptors+(sizeof(struct Descriptor)*tx_pointer)));
	if(desz->command!=0x80000064){ // a check if we somehow lost the count
		debugf("[RTL81] Unexpected default value: %x \n",desz->command);
	}
	desz->high_buf = ms4;
	desz->low_buf = ms3;
	desz->vlan = ms2;
	desz->command = ms1;
	
	((unsigned volatile long*)((unsigned volatile long)&package_send_ack))[0] = 0;
	outportb(bar1 + 0x38, 0x40); // ring the doorbell
	
	while(1){ // wait for int or end of polling
		unsigned volatile long x = ((unsigned volatile long*)((unsigned volatile long)&package_send_ack))[0];
		if(x==1){
			break;
		}
		unsigned volatile char poller = inportb(bar1 + 0x38);
		if((poller&0x40)==0){
			break;
		}
	}
	((unsigned volatile long*)((unsigned volatile long)&package_send_ack))[0] = 0;
}

PackageRecievedDescriptor rtl_recievePackage(){
	while(1){ // wait of arival of interrupt
		unsigned volatile long x = ((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0];
		if(x==1){
			break;
		}
	}
	((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0] = 0;
	struct Descriptor desc = Rx_Descriptors[rx_pointer++];
	PackageRecievedDescriptor res;
	unsigned long buffer_size = desc.command & 0x3FFF;
	res.buffersize = buffer_size;
	res.low_buf = desc.low_buf;
	res.high_buf = desc.high_buf;
	((unsigned volatile long*)((unsigned volatile long)&package_recieved_ack))[0] = 0;
	return res;
}

void rtl_test(){
	PackageRecievedDescriptor res = rtl_recievePackage();
	debugf("[RTL81] Testpackage recieved. Length=%x \n",res.buffersize);
	
	PackageRecievedDescriptor resx;
	resx.buffersize = res.buffersize;
	resx.low_buf = res.low_buf;
	resx.high_buf = 0;
	rtl_sendPackage(resx,1,1,0,0,0);
	debugf("[RTL81] Package send\n");
}

void init_rtl(int bus,int slot,int function){
	debugf("[RTL81] Driver loaded\n");
	bar1 = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFFE;
	debugf("[RTL81] BAR=%x \n",bar1);
	
	debugf("[RTL81] Set interrupter\n");
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	setNormalInt(usbint,(unsigned long)rtl8169irq);

	// enable device
	// outportb( bar1 + 0x52, 0x0);
	
	//
	// trigger reset
	debugf("[RTL81] Resetting driver \n");
	outportb(bar1 + 0x37, 0x10);
	while(inportb(bar1 + 0x37) & 0x10){}
	debugf("[RTL81] Driver reset succesfully \n");
	
	//
	// get mac address
	unsigned char macaddress[6];
	for(int i = 0 ; i < 6 ; i++){
		macaddress[i] = inportb(bar1+i);
	}
	debugf("[RTL81] MAC-address %x %x %x %x %x %x \n",macaddress[0],macaddress[1],macaddress[2],macaddress[3],macaddress[4],macaddress[5]);

	Rx_Descriptors = (struct Descriptor*)malloc_align(sizeof(struct Descriptor)*num_of_rx_descriptors,0xFFFF);
	Tx_Descriptors = (struct Descriptor*)malloc_align(sizeof(struct Descriptor)*num_of_tx_descriptors,0xFFFF);
	
	//
	// setup rx descriptor
	debugf("[RTL81] Setup RX descriptor\n");
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
	outportb(bar1 + 0xEC, 0x3B); /* max tx packet size */ // 0x3B
	outportl(bar1 + 0x20, (unsigned long)&Tx_Descriptors[0]); /* Tell the NIC where the first Tx descriptor is */
	outportl(bar1 + 0x24, (unsigned long)0); 
	outportl(bar1 + 0xE4, (unsigned long)&Rx_Descriptors[0]); /* Tell the NIC where the first Rx descriptor is */
	outportl(bar1 + 0xE8, (unsigned long)0);
	outportw(bar1 + 0x3C, 0xC1FF); /* Set all masks open so we get much ints */
	outportb(bar1 + 0x37, 0x0C); /* Enable Rx/Tx in the Command register */
	outportb(bar1 + 0x50, 0x00); /* Lock config registers */
	
	register_ethernet_device((unsigned long)&rtl_sendPackage,(unsigned long)&rtl_recievePackage,macaddress);
	debugf("[RTL81] Setup finished\n");
}
