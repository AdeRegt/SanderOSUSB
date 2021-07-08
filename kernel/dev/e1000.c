#include "../kernel.h"

struct e1000_rx_desc {
        unsigned long addr_1;
        unsigned long addr_2;
        unsigned short length;
        unsigned short checksum;
        unsigned char status;
        unsigned char errors;
        unsigned short special;
} __attribute__((packed));
 
struct e1000_tx_desc {
        unsigned long addr_1;
        unsigned long addr_2;
        unsigned short length;
        unsigned char cso;
        unsigned char cmd;
        unsigned char status;
        unsigned char css;
        unsigned short special;
} __attribute__((packed));

// from tutorial: https://wiki.osdev.org/Intel_Ethernet_i217

#define INTEL_VEND     0x8086  // Vendor ID for Intel 
#define E1000_DEV      0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217     0x153A  // Device ID for Intel I217
#define E1000_82577LM  0x10EA  // Device ID for Intel 82577LM
 
 
// I have gathered those from different Hobby online operating systems instead of getting them one by one from the manual
 
#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818
 
#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818
 
 
#define REG_RDTR         0x2820 // RX Delay Timer Register
#define REG_RXDCTL       0x3828 // RX Descriptor Control
#define REG_RADV         0x282C // RX Int. Absolute Delay Timer
#define REG_RSRPD        0x2C00 // RX Small Packet Detect Interrupt
 
 
 
#define REG_TIPG         0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU        0x40        //set link up
 
 
#define RCTL_EN                         (1 << 1)    // Receiver Enable
#define RCTL_SBP                        (1 << 2)    // Store Bad Packets
#define RCTL_UPE                        (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE                        (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE                        (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE                   (0 << 6)    // No Loopback
#define RCTL_LBM_PHY                    (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF                 (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER              (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH               (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36                      (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35                      (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34                      (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32                      (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM                        (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE                        (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN                      (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI                        (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF                        (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF                       (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC                      (1 << 26)   // Strip Ethernet CRC
 
// Buffer Sizes
#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))
 
 
// Transmit Command
 
#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable
 
 
// TCTL Register
 
#define TCTL_EN                         (1 << 1)    // Transmit Enable
#define TCTL_PSP                        (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT                   4           // Collision Threshold
#define TCTL_COLD_SHIFT                 12          // Collision Distance
#define TCTL_SWXOFF                     (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC                       (1 << 24)   // Re-transmit on Late Collision
 
#define TSTA_DD                         (1 << 0)    // Descriptor Done
#define TSTA_EC                         (1 << 1)    // Excess Collisions
#define TSTA_LC                         (1 << 2)    // Late Collision
#define LSTA_TU                         (1 << 3)    // Transmit Underrun

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

unsigned long base_addr;
unsigned char is_eeprom;
unsigned char mac_address[6];
unsigned volatile long e1000_package_recieved_ack = 0;
int rx_cur;
int tx_cur;
struct e1000_rx_desc *rx_descs[E1000_NUM_RX_DESC];
struct e1000_tx_desc *tx_descs[E1000_NUM_TX_DESC];

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
        debugf("[E1000] Link change!\n");
    }else if(to&0x80){
        debugf("[E1000] Package recieved!\n");
		((unsigned volatile long*)((unsigned volatile long)&e1000_package_recieved_ack))[0] = 1;
    }else if(to&0x10){
        debugf("[E1000] THG!\n");
    }else{
        debugf("[E1000] Unknown interrupt: %x !\n",to);
    }
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

void e1000_send_package(PackageRecievedDescriptor desc,unsigned char first,unsigned char last,unsigned char ip,unsigned char udp, unsigned char tcp){
    tx_descs[tx_cur]->addr_1 = (unsigned long)desc.low_buf;
    tx_descs[tx_cur]->addr_2 = (unsigned long)desc.high_buf;
    tx_descs[tx_cur]->length = desc.buffersize;
    tx_descs[tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx_descs[tx_cur]->status = 0;
    unsigned char old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
    e1000_write_in_space(REG_TXDESCTAIL, tx_cur);   
    while(!(tx_descs[old_cur]->status & 0xff));    
    debugf(" Stuff sended! (loc=%x, %x %x %x %x %x)\n",desc.low_buf,first,last,ip,udp,tcp);
    return;
}

void e1000_enable_int(){
    e1000_write_in_space(0xD0,0x1F6DC);
    e1000_write_in_space(0xD0,0xff & ~4);
    e1000_read_in_space(0xC0);
    debugf("[E1000] Interrupts enabled!\n");
}

void e1000_link_up(){
    unsigned long ty = e1000_read_in_space(0);
    e1000_write_in_space(0, ty | 0x40);
    debugf("[E1000] Link is up!\n");
}

PackageRecievedDescriptor e1000_recieve_package(){
    ((unsigned volatile long*)((unsigned volatile long)&e1000_package_recieved_ack))[0] = 0;
	while(1){ // wait of arival of interrupt
		unsigned volatile long x = ((unsigned volatile long*)((unsigned volatile long)&e1000_package_recieved_ack))[0];
		if(x==1){
			break;
		}
	}

    PackageRecievedDescriptor prd;
    unsigned short old_cur;

    if((rx_descs[rx_cur]->status & 0x1))
    {
        unsigned char *buf = (unsigned char *)rx_descs[rx_cur]->addr_1;
        unsigned short len = rx_descs[rx_cur]->length;
        prd.buffersize = len;
        prd.high_buf = 0;
        prd.low_buf = (unsigned long)buf;

        rx_descs[rx_cur]->status = 0;
        old_cur = rx_cur;
        rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
        e1000_write_in_space(REG_RXDESCTAIL, old_cur );
    }
	return prd;
}

void init_e1000(int bus,int slot,int function){
    debugf("[E1000] E1000 initialised bus=%x slot=%x function=%x \n",bus,slot,function);
    base_addr = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFFE;
    debugf("[E1000] Base address: %x \n",base_addr);
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
    debugf("[E1000] USBINT %x \n",usbint);
    setNormalInt(usbint,(unsigned long)e1000irq);

    //
    // mac ophalen
    is_eeprom = e1000_is_eeprom();
    if(is_eeprom){
        debugf("[E1000] Device has EEPROM\n");
        debugf("[E1000] EEPROM is not supported, yet\n");
        return;
    }else{
        debugf("[E1000] Device has NO EEPROM\n");
        unsigned long *tg = (unsigned long*) (base_addr + 0x5400);
        mac_address[0] = ((tg[0] & 0x000000FF)>>0) & 0xFF;
        mac_address[1] = ((tg[0] & 0x0000FF00)>>8) & 0xFF;
        mac_address[2] = ((tg[0] & 0x00FF0000)>>16) & 0xFF;
        mac_address[3] = ((tg[0] & 0xFF000000)>>24) & 0xFF;
        mac_address[4] = ((tg[1] & 0x000000FF)>>0) & 0xFF;
        mac_address[5] = ((tg[1] & 0x0000FF00)>>8) & 0xFF;
    }
    debugf("[E1000] MAC: %x:%x:%x:%x:%x:%x \n",mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5]);

    if(!(getBARaddress(bus,slot,function,0x04)&0x04)){
        unsigned long to = pciConfigReadWord(bus,slot,function,0x04) | 0x04;
        pciConfigWriteWord(bus,slot,function,0x04,to);
        debugf("[E1000] Busmastering was not enabled, but now it is!\n");
    }


    // no multicast
    for(int i = 0; i < 0x80; i++){
        e1000_write_in_space(0x5200 + (i * 4), 0);
    }

    //
    // set reciever
    unsigned char * ptr;
    struct e1000_rx_desc *descs;
 
    ptr = (unsigned char *)(malloc(sizeof(struct e1000_rx_desc)*E1000_NUM_RX_DESC + 16));
 
    descs = (struct e1000_rx_desc *)ptr;
    for(int i = 0; i < E1000_NUM_RX_DESC; i++)
    {
        rx_descs[i] = (struct e1000_rx_desc *)((unsigned char *)descs + i*16);
        rx_descs[i]->addr_1 = (unsigned long)(unsigned char *)(malloc(8192 + 16));
        rx_descs[i]->status = 0;
    }
 
    e1000_write_in_space(REG_TXDESCLO, (unsigned long) ptr);
    e1000_write_in_space(REG_TXDESCHI, 0);
 
    e1000_write_in_space(REG_RXDESCLO, (unsigned long)ptr);
    e1000_write_in_space(REG_RXDESCHI, 0);
 
    e1000_write_in_space(REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);
 
    e1000_write_in_space(REG_RXDESCHEAD, 0);
    e1000_write_in_space(REG_RXDESCTAIL, E1000_NUM_RX_DESC-1);
    rx_cur = 0;
    e1000_write_in_space(REG_RCTRL, RCTL_EN| RCTL_SBP| RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC  | RCTL_BSIZE_8192);
    
    //
    // sending
    unsigned char *  ptr2;
    struct e1000_tx_desc *descs2;
    ptr2 = (unsigned char *)(malloc(sizeof(struct e1000_tx_desc)*E1000_NUM_TX_DESC + 16));
 
    descs2 = (struct e1000_tx_desc *)ptr2;
    for(int i = 0; i < E1000_NUM_TX_DESC; i++)
    {
        tx_descs[i] = (struct e1000_tx_desc *)((unsigned char*)descs2 + i*16);
        tx_descs[i]->addr_1 = 0;
        tx_descs[i]->cmd = 0;
        tx_descs[i]->status = TSTA_DD;
    }
 
    e1000_write_in_space(REG_TXDESCHI, (unsigned long)ptr2 );
    e1000_write_in_space(REG_TXDESCLO, 0);
 
    e1000_write_in_space(REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);
 
    e1000_write_in_space( REG_TXDESCHEAD, 0);
    e1000_write_in_space( REG_TXDESCTAIL, 0);
    tx_cur = 0;
    e1000_write_in_space(REG_TCTRL,  TCTL_EN
        | TCTL_PSP
        | (15 << TCTL_CT_SHIFT)
        | (64 << TCTL_COLD_SHIFT)
        | TCTL_RTLC);
 
    e1000_write_in_space(REG_TCTRL,  0b0110000000000111111000011111010);
    e1000_write_in_space(REG_TIPG,  0x0060200A);

    //
    // set interrupts
    e1000_enable_int();
    e1000_link_up();

    //
    // register driver
    register_ethernet_device((unsigned long)&e1000_send_package,(unsigned long)&e1000_recieve_package);
}