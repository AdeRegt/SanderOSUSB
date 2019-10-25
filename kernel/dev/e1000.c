#include "../kernel.h"

unsigned long BARE1000 = 0;
unsigned char mac[6];
unsigned char useseprom = 0;
unsigned long rx_descs_phys;
unsigned long tx_descs_phys;

unsigned long E1000ReadCommand (unsigned short addr){
	return *( (volatile unsigned int *) (BARE1000 + addr));
}

void E1000WriteCommand (unsigned short addr, unsigned long val ){
	*( (volatile unsigned int *)(BARE1000 + addr)) = val;
}

extern void e1000irq();
void irq_e1000(){
	
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

// code modified from https://github.com/frednora/gramado/blob/master/kernel/kdrivers/network/nicintel.c
void e1000_init(int bus,int slot,int function){
	unsigned long data = (unsigned long) pciConfigReadDWord ( bus, slot, function, 0 );
	unsigned short Vendor = (unsigned short) (data       & 0xffff);
	unsigned short Device = (unsigned short) (data >> 16 & 0xffff);
	if(!(Vendor != 0x8086 || Device != 0x100E)){
		setNormalInt(pciConfigReadWord ( bus, slot, function, 0x3c ) & 0xFF,(unsigned long)e1000irq);
		BARE1000 = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFF0;
		unsigned char *base_address = (unsigned char *) BARE1000;
		printf("E1000: compatible device at %x with status %x \n",BARE1000,base_address[0x8]);
		for(int i = 0 ; i < 1000 ; i++){
			unsigned long val = ((unsigned long*)BARE1000+0x14)[0];
			if ( (val & 0x10) == 0x10) {
				useseprom = 1;
				printf("E1000: uses eeprom\n");
			}
		}
		if(useseprom){
			printf("E1000: eeprom not supported!\n");
			for(;;);
		}else{
			mac[0] = base_address[ 0x5400 + 0 ];
			mac[1] = base_address[ 0x5400 + 1 ];
			mac[2] = base_address[ 0x5400 + 2 ];
			mac[3] = base_address[ 0x5400 + 3 ];
			mac[4] = base_address[ 0x5400 + 4 ];
			mac[5] = base_address[ 0x5400 + 5 ];
		}
		printf("E1000: mac is %x:%x:%x:%x:%x:%x \n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		
		for(int i = 0 ; i < 0x80 ; i++){
			E1000WriteCommand(0x5200+(i*4),0);
		}
		tx_descs_phys = malloc(0x1000);
		rx_descs_phys = malloc(0x1000);
		
		E1000WriteCommand (0xD0, 0x1F6DC);
		E1000ReadCommand (0xC0);
		E1000WriteCommand (0x2800, rx_descs_phys );
		E1000WriteCommand (0x2804, 0);
		E1000WriteCommand (0x2808, 512);
		E1000WriteCommand (0x2810, 0);
		E1000WriteCommand (0x2818, 31);
		E1000WriteCommand (0x100, 0x602801E);
		E1000WriteCommand (0x3800, tx_descs_phys );					
		E1000WriteCommand (0x3804, 0);
		E1000WriteCommand (0x3808, 128);
		E1000WriteCommand (0x3810, 0);
		E1000WriteCommand (0x3818, 0);
    		E1000WriteCommand (0x3828, (0x01000000 | 0x003F0000)); 
    		E1000WriteCommand (0x400, ( 0x00000ff0 | 0x003ff000 | 0x00000008 | 0x00000002) );	
	 	E1000WriteCommand (0x410, (  0x0000000A | 0x00000008 | 0x00000002) );
		unsigned long val = E1000ReadCommand (0);
		E1000WriteCommand (0, val | 0x40);
		for(;;);
	}
}
