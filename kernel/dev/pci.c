#include "../kernel.h"

//
// PCI
//
//
#define PCI_ADDRESS 0xCF8
#define PCI_DATA 0xCFC

unsigned long pciConfigReadDWord (unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned long address;
    unsigned long lbus  = (unsigned long)bus;
    unsigned long lslot = (unsigned long)slot;
    unsigned long lfunc = (unsigned long)func;
 
    /* create configuration address as per Figure 1 */
    address = (unsigned long)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
 
    /* write out the address */
    outportl(PCI_ADDRESS, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    return (inportl(PCI_DATA) >> ((offset & 2) * 8));
}

unsigned short pciConfigReadWord (unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned long address;
    unsigned long lbus  = (unsigned long)bus;
    unsigned long lslot = (unsigned long)slot;
    unsigned long lfunc = (unsigned long)func;
    unsigned short tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (unsigned long)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
 
    /* write out the address */
    outportl(PCI_ADDRESS, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (unsigned short)((inportl(PCI_DATA) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

unsigned long getBARaddress(int bus,int slot,int function,int barNO){
	unsigned long result = 0;
	unsigned long partA = pciConfigReadWord(bus,slot,function,barNO);
	unsigned long partB = pciConfigReadWord(bus,slot,function,barNO+2);
	result = ((partB<<16) | ((partA) & 0xffff));
	return result;
}

void init_pci(){
	printstring("PCI: detecting devices....\n");
	for(int bus = 0 ; bus < 256 ; bus++){
		for(int slot = 0 ; slot < 32 ; slot++){
			for(int function = 0 ; function < 7 ; function++){
				unsigned short vendor = pciConfigReadWord(bus,slot,function,0);
				if(vendor != 0xFFFF){
					printstring("PCI: device detected, ");
					unsigned char classc = (pciConfigReadWord(bus,slot,function,0x0A)>>8)&0xFF;
					unsigned char sublca = (pciConfigReadWord(bus,slot,function,0x0A))&0xFF;
					unsigned char subsub = (pciConfigReadWord(bus,slot,function,0x08)>>8)&0xFF;
					unsigned short subsystemvendor = pciConfigReadWord(bus,slot,function,0x2c) & 0xFFFF;
					unsigned short subsystemid = pciConfigReadWord(bus,slot,function,0x2e) & 0xFFFF;
					if(subsystemvendor==0x5006||subsystemid==0x5006){
						printf("FOUND IT");for(;;);
					}
					printf("subvend %x subid %x ",subsystemvendor,subsystemid);
					if(classc==0x00){
						printstring("unclassified: ");
					}else if(classc==0x01){
						printstring("mass storage device: ");
						if(sublca==0x00){
							printstring(" SCSI Bus");
						}else if(sublca==0x01){
							printstring(" IDE controller\n");
							unsigned short base = pciConfigReadWord(bus,slot,function,0x20);
							init_ide(base);
						}else if(sublca==0x02){
							printstring(" Floppy disk");
						}else if(sublca==0x03){
							printstring(" IPI Bus");
						}else if(sublca==0x04){
							printstring(" RAID Controller");
						}else if(sublca==0x05){
							printstring(" ATA Controller");
						}else if(sublca==0x06){
							printstring(" Serial ATA\n");
							ahci_init(bus,slot,function);
						}else if(sublca==0x07){
							printstring(" Serial attached SCSI");
						}else if(sublca==0x08){
							printstring(" Non-volatile memory controller");
						}else{
							printstring("UNDEFINED");
						}
					}else if(classc==0x02){
						printstring("network controller: ");
						if(sublca==0x00){
							printstring(" Ethernet controller\n");
							e1000_init(bus,slot,function);
						}else if(sublca==0x01){
							printstring(" Token ring controller");
						}else if(sublca==0x02){
							printstring(" FDDI controller");
						}else if(sublca==0x03){
							printstring(" ATM controller");
						}else if(sublca==0x04){
							printstring(" ISDN controller");
						}else if(sublca==0x05){
							printstring(" WorldFlip controller");
						}else if(sublca==0x06){
							printstring(" PCMG controller");
						}else if(sublca==0x07){
							printstring(" Infiniband controller");
						}else if(sublca==0x08){
							printstring(" Fabric controller");
						}else if(sublca==0x80){
							printstring(" Other controller");
						}
					}else if(classc==0x03){
						printstring("displaycontroller: ");
						if(sublca==0x00){
							printstring(" VGA controller");
						}else if(sublca==0x01){
							printstring(" XGA controller");
						}else if(sublca==0x02){
							printstring(" 3D controller");
						}else if(sublca==0x80){
							printstring(" Other controller");
						}
					}else if(classc==0x04){
						printstring("multimedia controller: ");
					}else if(classc==0x05){
						printstring("memory controller: ");
						if(sublca==0x00){
							printstring(" RAM controller");
						}else if(sublca==0x01){
							printstring(" FLASH controller");
						}else if(sublca==0x80){
							printstring(" Other controller");
						}
					}else if(classc==0x06){
						printstring("bridge device: ");
						if(sublca==0x00){
							printstring(" host bridge");
						}else if(sublca==0x01){
							printstring(" ISA bridge");
						}else if(sublca==0x02){
							printstring(" EISA bridge");
						}else if(sublca==0x03){
							printstring(" MCA");
						}else if(sublca==0x04){
							printstring(" PCI to PCI bridge");
						}else if(sublca==0x05){
							printstring(" PCMCIA bridge");
						}else if(sublca==0x06){
							printstring(" NuBus bridge");
						}else if(sublca==0x07){
							printstring(" CardBus bridge");
						}else if(sublca==0x08){
							printstring(" RACEWay bridge");
						}else if(sublca==0x09){
							printstring(" PCI to PCI bridge");
						}else if(sublca==0x0A){
							printstring(" Infiniband to PCI bridge");
						}else if(sublca==0x80){
							printstring(" bridge");
						}
					}else if(classc==0x07){
						printstring("simple communication controller: ");
					}else if(classc==0x08){
						printstring("base system peripel: ");
					}else if(classc==0x09){
						printstring("inputdevice: ");
					}else if(classc==0x0A){
						printstring("docking station: ");
					}else if(classc==0x0B){
						printstring("processor: ");
					}else if(classc==0x0C){
						printstring("serial buss controller: ");
						if(sublca==0x00){
							printstring(" FireWire controller");
						}else if(sublca==0x01){
							printstring(" Access controller");
						}else if(sublca==0x02){
							printstring(" SSA controller");
						}else if(sublca==0x03){
							printstring(" USB controller, ");
							if(subsub==0x00){
								printstring("UHCI [USB 1]");
							}else if(subsub==0x10){
								printstring("OHCI [USB 1]");
							}else if(subsub==0x20){
								printstring("EHCI [USB 2]");
							}else if(subsub==0x30){
								printstring("XHCI [USB 3]\n");
//								unsigned long bar1 = getBARaddress(bus,slot,function,0x10);
//								unsigned long bar2 = getBARaddress(bus,slot,function,0x14);
//								unsigned long capabilityregs = bar1+(getBARaddress(bus,slot,function,0x34) & 0xFF);
								
								//init_xhci(bar1,bar2,capabilityregs);
							}else if(subsub==0x80){
								printstring("unspecified");
							}else if(subsub==0xFE){
								printstring("devicecontroller");
							}else{
								printstring("unknown");
							}
						}else if(sublca==0x04){
							printstring(" fibre controller");
						}else if(sublca==0x05){
							printstring(" SMBus controller");
						}else if(sublca==0x06){
							printstring(" Infiniband controller");
						}else if(sublca==0x07){
							printstring(" IPMI controller");
						}else if(sublca==0x08){
							printstring(" SERCOS controller");
						}else if(sublca==0x09){
							printstring(" CANbus controller");
						}else if(sublca==0x80){
							printstring(" Other controller");
						}
					}else if(classc==0x0D){
						printstring("wireless controller: ");
					}else if(classc==0x0E){
						printstring("inteligent controller: ");
					}else if(classc==0x0F){
						printstring("satalite controller: ");
					}else if(classc==0x10){
						printstring("encryption controller: ");
					}else if(classc==0x11){
						printstring("signal controller: ");
					}else if(classc==0x12){
						printstring("accelerator controller: ");
					}else if(classc==0x13){
						printstring("non essential controller: ");
					}else if(classc==0x40){
						printstring("co processor controller: ");
					}else if(classc==0xFF){
						printstring("unassigned controller: ");
					}else{
						printstring("UNKNOWN");
					}
					printstring("\n");
					if(vendor==0x80EE){
						printstring("VBOX: Guestadditions found!!\n");
						//unsigned long bx = getBARaddress(bus,slot,function,0x10);
						//unsigned char tx = pciConfigReadWord(bus,slot,function,0x3C)&0xFF;
						//init_vbox(bx,tx);
					}
				}
			}
		}
	}
}

