//
// includes.... everything in one file for simplicity
#include "../kernel.h"

void ethernet_detect(int bus,int slot,int function,int device,int vendor){
    if(device==0x8168&&vendor==0x10ec){ 
        // Sander his RTL8169 driver comes here
        init_rtl(bus,slot,function);
    }else{
        // Johan his E1000 driver comes here
    }
}

EthernetDevice defaultEthernetDevice;

void register_ethernet_device(unsigned long sendPackage,unsigned long recievePackage){
    defaultEthernetDevice.recievePackage = recievePackage;
    defaultEthernetDevice.sendPackage = sendPackage;
}

EthernetDevice getDefaultEthernetDevice(){
    return defaultEthernetDevice;
}

PackageRecievedDescriptor getEthernetPackage(){
    PackageRecievedDescriptor (*getPackage)() = (void*)defaultEthernetDevice.recievePackage;
    return getPackage();
}

void sendEthernetPackage(PackageRecievedDescriptor desc,unsigned char first,unsigned char last){
    void (*sendPackage)(PackageRecievedDescriptor desc,unsigned char first,unsigned char last) = (void*)defaultEthernetDevice.sendPackage;
    sendPackage(desc,first,last);
}