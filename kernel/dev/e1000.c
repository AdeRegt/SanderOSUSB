#include "../kernel.h"

struct e1000_rx_desc {
        volatile unsigned long addr_1;
        volatile unsigned long addr_2;
        volatile unsigned short length;
        volatile unsigned short checksum;
        volatile unsigned char status;
        volatile unsigned char errors;
        volatile unsigned short special;
} __attribute__((packed));
 
struct e1000_tx_desc {
        volatile unsigned long addr_1;
        volatile unsigned long addr_2;
        volatile unsigned short length;
        volatile unsigned char cso;
        volatile unsigned char cmd;
        volatile unsigned char status;
        volatile unsigned char css;
        volatile unsigned short special;
} __attribute__((packed));

unsigned long base_addr;
unsigned char is_eeprom;
unsigned char mac_address[6];
struct e1000_rx_desc *recieved;
struct e1000_tx_desc *sended;

extern void e1000irq();

void e1000_write_in_space(unsigned short addr,unsigned long val){
    *( (volatile unsigned int *)(base_addr + addr)) = val;
}

unsigned long e1000_read_in_space(unsigned short addr){
    return *( (volatile unsigned int *)(base_addr + addr));
}

unsigned int e1000_read_command(unsigned short addr){
    unsigned long adt = base_addr;
    unsigned int *t = (unsigned int*) adt;
    return t[addr];
}

unsigned char e1000_is_eeprom(){
    for(int i = 0 ; i < 1000 ; ++i){
        unsigned long to = e1000_read_command(0x14);
        if((to&0x10)==0x10){
            return 1;
        }
    }
    return 0;
}

void irq_e1000(){
    e1000_write_in_space(0xD0,1);
    unsigned long to = e1000_read_in_space(0xC0);
	if(to&0x04){
        printf("[E1000] Link change!\n");
    }else if(to&0x80){
        printf("[E1000] Package recieved!\n");
        for(int rx_cur = 0 ; rx_cur < 10 ; rx_cur++){
            if(recieved[rx_cur].status & 0x1){
                printf("[E1000] Package found!\n");
            }
        }
    }else{
        printf("[E1000] Unknown interrupt!\n");
    }
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

void init_e1000(int bus,int slot,int function){
    printf("[E1000] E1000 initialised bus=%x slot=%x function=%x \n",bus,slot,function);
    base_addr = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFFE;
    printf("[E1000] Base address: %x \n",base_addr);
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
    printf("[E1000] USBINT %x \n",usbint);
    setNormalInt(usbint,(unsigned long)e1000irq);

    //
    // mac ophalen
    is_eeprom = e1000_is_eeprom();
    if(is_eeprom){
        printf("[E1000] Device has EEPROM\n");
        printf("[E1000] EEPROM is not supported, yet\n");
        return;
    }else{
        printf("[E1000] Device has NO EEPROM\n");
        unsigned long *tg = (unsigned long*) (base_addr + 0x5400);
        mac_address[0] = ((tg[0] & 0x000000FF)>>0) & 0xFF;
        mac_address[1] = ((tg[0] & 0x0000FF00)>>8) & 0xFF;
        mac_address[2] = ((tg[0] & 0x00FF0000)>>16) & 0xFF;
        mac_address[3] = ((tg[0] & 0xFF000000)>>24) & 0xFF;
        mac_address[4] = ((tg[1] & 0x000000FF)>>0) & 0xFF;
        mac_address[5] = ((tg[1] & 0x0000FF00)>>8) & 0xFF;
    }
    printf("[E1000] MAC: %x:%x:%x:%x:%x:%x \n",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);
    recieved = (struct e1000_rx_desc*)malloc_align(0x1000,0xFFF);
    sended = (struct e1000_tx_desc*)malloc_align(0x1000,0xFFF);

    // fill recieved
    for(int i = 0 ; i < 10 ; i++){
        recieved[i].addr_1 = malloc(8192 + 16);
    }

    //
    // set interrupts
    e1000_write_in_space(0xD0,0x1F6DC);
    e1000_read_in_space(0xC0);

    //
    // set reciever
    e1000_write_in_space(0x2800, (unsigned long) recieved); // ring addr
    e1000_write_in_space(0x2804, 0);
    e1000_write_in_space(0x2808, 512);
    e1000_write_in_space(0x2810, 0);
    e1000_write_in_space(0x2818, 31);
    e1000_write_in_space(0x100, 0x602801E);

    //
    // set sender
    e1000_write_in_space(0x3800, (unsigned long) sended); // ring addr
    e1000_write_in_space(0x3804, 0);
    e1000_write_in_space(0x3808, 128);
    e1000_write_in_space(0x3810, 0);
    e1000_write_in_space(0x3818, 7);

    e1000_write_in_space(0x3828, (0x01000000 | 0x003F0000));
    e1000_write_in_space(0x400, ( 0x00000ff0 | 0x003ff000 | 0x00000008 | 0x00000002));
    e1000_write_in_space(0x410, (  0x0000000A | 0x00000008 | 0x00000002));

    unsigned long ty = e1000_read_in_space(0);
    e1000_write_in_space(0, ty | 0x40);

    //
    // register driver
    for(;;);
}