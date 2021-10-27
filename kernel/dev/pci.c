#include "../kernel.h"

//
// PCI
//
//
#define PCI_ADDRESS 0xCF8
#define PCI_DATA 0xCFC

void dumpPCI(int bus,int slot,int function){
	printf("========================\n");
	printf("PCI DUMP\n");
	printf("------------------------\n");
	printf("Vendor ID  = %x \n",getBARaddress(bus,slot,function,0) & 0x0000FFFF);
	printf("Device ID  = %x \n",(getBARaddress(bus,slot,function,0) & 0xFFFF0000) >> 16);
	
	printf("Command    = %x \n", getBARaddress(bus,slot,function,0x04) & 0x0000FFFF);
	printf("Status     = %x \n",(getBARaddress(bus,slot,function,0x04) & 0xFFFF0000) >> 16);
	
	printf("RevisionID = %x \n", getBARaddress(bus,slot,function,0x08) & 0x000000FF);
	printf("Prog IF    = %x \n",(getBARaddress(bus,slot,function,0x08) & 0x0000FF00)>> 8);
	printf("Subclass   = %x \n",(getBARaddress(bus,slot,function,0x08) & 0x00FF0000)>> 16);
	printf("Classcode  = %x \n",(getBARaddress(bus,slot,function,0x08) & 0xFF000000)>> 24);
	
	printf("Cache Line = %x \n", getBARaddress(bus,slot,function,0x0C) & 0x000000FF);
	printf("Latency Tim= %x \n",(getBARaddress(bus,slot,function,0x0C) & 0x0000FF00)>> 8);
	printf("Header type= %x \n",(getBARaddress(bus,slot,function,0x0C) & 0x00FF0000)>> 16);
	printf("BIST       = %x \n",(getBARaddress(bus,slot,function,0x0C) & 0xFF000000)>> 24);
	
	printf("BAR        = %x %x %x %x %x %x \n", 
		getBARaddress(bus,slot,function,0x10), 
		getBARaddress(bus,slot,function,0x14), 
		getBARaddress(bus,slot,function,0x18), 
		getBARaddress(bus,slot,function,0x1C), 
		getBARaddress(bus,slot,function,0x20), 
		getBARaddress(bus,slot,function,0x24)
	);
	
	printf("Cardbus CIS= %x \n",getBARaddress(bus,slot,function,0x28));
	
	printf("Subsystem V= %x \n",getBARaddress(bus,slot,function,0x2C) & 0x0000FFFF);
	printf("Subsystem  = %x \n",(getBARaddress(bus,slot,function,0x2C) & 0xFFFF0000) >> 16);
	
	printf("Expansion R= %x \n",getBARaddress(bus,slot,function,0x30));
	
	printf("Capability = %x \n", getBARaddress(bus,slot,function,0x34) & 0x000000FF);
	
	printf("Interrupt L= %x \n", getBARaddress(bus,slot,function,0x3C) & 0x000000FF);
	printf("Interrupt P= %x \n",(getBARaddress(bus,slot,function,0x3C) & 0x0000FF00)>> 8);
	printf("Min Grant  = %x \n",(getBARaddress(bus,slot,function,0x3C) & 0x00FF0000)>> 16);
	printf("Max latency= %x \n",(getBARaddress(bus,slot,function,0x3C) & 0xFF000000)>> 24);
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

void pciConfigWriteWord (unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset,unsigned long value) {
    unsigned long address;
    unsigned long lbus  = (unsigned long)bus;
    unsigned long lslot = (unsigned long)slot;
    unsigned long lfunc = (unsigned long)func;
 
    /* create configuration address as per Figure 1 */
    address = (unsigned long)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
 
    /* write out the address */
    outportl(PCI_ADDRESS, address);
    outportl(PCI_DATA,value);
}

void setBARaddress(int bus,int slot,int function,int barNO,unsigned long result){
	unsigned long partA = result & 0xffff;
	unsigned long partB = (result>>16) & 0xffff;
	pciConfigWriteWord(bus,slot,function,barNO+0,partA);
	pciConfigWriteWord(bus,slot,function,barNO+2,partB);
}

unsigned long getBARaddress(int bus,int slot,int function,int barNO){
	unsigned long result = 0;
	unsigned long partA = pciConfigReadWord(bus,slot,function,barNO);
	unsigned long partB = pciConfigReadWord(bus,slot,function,barNO+2);
	result = ((partB<<16) | ((partA) & 0xffff));
	return result;
}

void dumpPCIConfiguration(int bus,int slot,int function){
	unsigned long config = getPCIConfiguration(bus,slot,function);
	printf("[PCI] Config flags: ");
	if(config & 0b0000000000000001){
		printf(" I/O");
	}
	if(config & 0b0000000000000010){
		printf(" Memory");
	}
	if(config & 0b0000000000000100){
		printf(" Master");
	}
	if(config & 0b0000000000001000){
		printf(" Special");
	}
	printf("\n");
}

unsigned long getPCIConfiguration(int bus,int slot,int function){
	return getBARaddress(bus,slot,function,0x04) & 0x0000FFFF;
}

char pci_enable_busmastering_when_needed(int bus,int slot,int function){
	if(!(getPCIConfiguration(bus,slot,function)&0x04)){
        printf("[PCI] PCI bussmaster not enabled!\n");
        unsigned long setting = getPCIConfiguration(bus,slot,function);
        setting |= 0x04;
        setBARaddress(bus,slot,function,4,setting);
        if(!(getPCIConfiguration(bus,slot,function)&0x04)){
            printf("[PCI] PCI busmastering still not enabled! Quiting...\n");
            return 0;
        }else{
            printf("[PCI] PCI busmastering is now enabled.\n");
        }
    }
	return 1;
}

void init_pci(){
	printstring("PCI: detecting devices....\n");
	for(int bus = 0 ; bus < 256 ; bus++){
		for(int slot = 0 ; slot < 32 ; slot++){
			for(int function = 0 ; function <= 7 ; function++){
				unsigned short vendor = pciConfigReadWord(bus,slot,function,0);
				unsigned short device = pciConfigReadWord(bus,slot,function,2);
				if(vendor != 0xFFFF){
					printstring("PCI: device detected, ");
					unsigned char classc = (pciConfigReadWord(bus,slot,function,0x0A)>>8)&0xFF;
					unsigned char sublca = (pciConfigReadWord(bus,slot,function,0x0A))&0xFF;
					unsigned char subsub = (pciConfigReadWord(bus,slot,function,0x08)>>8)&0xFF;
					//unsigned short subsystemvendor = pciConfigReadWord(bus,slot,function,0x2c) & 0xFFFF;
					//unsigned short subsystemid = pciConfigReadWord(bus,slot,function,0x2e) & 0xFFFF;
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
							printf(" Ethernet controller device:%x vendor:%x \n",device,vendor);
							ethernet_detect(bus,slot,function,device,vendor);
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
						}else if(sublca==0x03&&getGrubStatus().usb==0){
							printstring(" USB controller, ");
							if(subsub==0x00){
								printstring("UHCI [USB 1]\n");
								//uhci_init(bus,slot,function);
							}else if(subsub==0x10){
								printstring("OHCI [USB 1]");
							}else if(subsub==0x20){
								printstring("EHCI [USB 2]\n");
								init_ehci(bus,slot,function);
							}else if(subsub==0x30){
								printstring("XHCI [USB 3]\n");
								init_xhci(bus,slot,function);
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
					if(vendor==0x80EE&&device==0xCAFE){
						init_vbox(bus,slot,function);
					}
				}
			}
		}
	}
}

