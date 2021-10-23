//
// includes.... everything in one file for simplicity
#include "../kernel.h"

void ethernet_detect(int bus,int slot,int function,int device,int vendor){
    if((device==0x8168||device==0x8139)&&vendor==0x10ec){ 
        // Sander his RTL8169 driver comes here
        init_rtl(bus,slot,function);
    }else if(device==0x100e||device==0x153A||device==0x10EA||vendor==0x8086){
        // Johan his E1000 driver comes here
        init_e1000(bus,slot,function);
    }else{
        printf("[ETH] Unknown ethernet device: device: %x vendor: %x \n",device,vendor);
    }
}

EthernetDevice defaultEthernetDevice;

void register_ethernet_device(unsigned long sendPackage,unsigned long recievePackage,unsigned char mac[8]){
    defaultEthernetDevice.recievePackage = recievePackage;
    defaultEthernetDevice.sendPackage = sendPackage;
    defaultEthernetDevice.is_enabled = 1;
    for(int i = 0 ; i < 8 ; i++){
        defaultEthernetDevice.mac[i] = mac[i];
    }
}

EthernetDevice getDefaultEthernetDevice(){
    return defaultEthernetDevice;
}

PackageRecievedDescriptor getEthernetPackage(){
    PackageRecievedDescriptor (*getPackage)() = (void*)defaultEthernetDevice.recievePackage;
    return getPackage();
}

void sendEthernetPackage(PackageRecievedDescriptor desc,unsigned char first,unsigned char last,unsigned char ip,unsigned char udp, unsigned char tcp){
    void (*sendPackage)(PackageRecievedDescriptor desc,unsigned char first,unsigned char last,unsigned char ip,unsigned char udp, unsigned char tcp) = (void*)defaultEthernetDevice.sendPackage;
    sendPackage(desc,first,last,ip,udp,tcp);
}

#define SIZE_OF_MAC 6
#define SIZE_OF_IP 4
#define ETHERNET_TYPE_ARP 0x0608

struct EthernetHeader{
    unsigned char to[SIZE_OF_MAC];
    unsigned char from[SIZE_OF_MAC];
    unsigned short type;
} __attribute__ ((packed));

struct ARPHeader{
    struct EthernetHeader ethernetheader;
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_address_length;
    unsigned char protocol_address_length;
    unsigned short operation;

    unsigned char source_mac[SIZE_OF_MAC];
    unsigned char source_ip[SIZE_OF_IP];

    unsigned char dest_mac[SIZE_OF_MAC];
    unsigned char dest_ip[SIZE_OF_IP];
} __attribute__ ((packed));

void fillMac(unsigned char* to,unsigned char* from){
    for(int i = 0 ; i < SIZE_OF_MAC ; i++){
        to[i] = from[i];
    }
}

void fillIP(unsigned char* to,unsigned char* from){
    for(int i = 0 ; i < SIZE_OF_IP ; i++){
        to[i] = from[i];
    }
}

void fillEthernetHeader(struct EthernetHeader* eh, unsigned char* destip,unsigned short type){
    fillMac((unsigned char*)&eh->to,destip);
    fillMac((unsigned char*)&eh->from,(unsigned char*)&defaultEthernetDevice.mac);
    eh->type = type;
}

unsigned char* getMACFromIp(unsigned char* ip){
    struct ARPHeader* arpie = (struct ARPHeader*)malloc(sizeof(struct ARPHeader));
    unsigned char everyone[SIZE_OF_MAC] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    unsigned char empty[SIZE_OF_MAC] = {0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char prefip[SIZE_OF_IP] = {192,168,5,5};
    fillEthernetHeader((struct EthernetHeader*) &arpie->ethernetheader,everyone,ETHERNET_TYPE_ARP);
    arpie->hardware_type = 0x0100;
    arpie->protocol_type = 0x0008;
    arpie->hardware_address_length = SIZE_OF_MAC;
    arpie->protocol_address_length = SIZE_OF_IP;
    arpie->operation = 0x0100;

    fillMac((unsigned char*)&arpie->source_mac,(unsigned char*)&defaultEthernetDevice.mac);
    fillIP((unsigned char*)&arpie->source_ip,(unsigned char*)&prefip);

    fillMac((unsigned char*)&arpie->dest_mac,(unsigned char*)&empty);
    fillIP((unsigned char*)&arpie->dest_ip,ip);
    
    PackageRecievedDescriptor sec;
    sec.buffersize = sizeof(struct ARPHeader);
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)arpie;

    sleep(10);
    sendEthernetPackage(sec,1,1,1,0,0);
    sleep(10);
    PackageRecievedDescriptor prd = getEthernetPackage();
    sleep(10);
    struct ARPHeader* ah = (struct ARPHeader*) prd.low_buf;
    return ah->source_mac;
}



void initialise_ethernet(){
    printf("[ETH] Ethernet module reached!\n");
    EthernetDevice ed = getDefaultEthernetDevice();
    if(ed.is_enabled){
        printf("[ETH] There is a ethernet device present on the system!\n");
    }
}