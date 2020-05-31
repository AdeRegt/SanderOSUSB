#include "../kernel.h"
// cd ../kernel
// bash build.sh 
// ../../qemu/x86_64-softmmu/qemu-system-x86_64 --cdrom ../cdrom.iso  -trace enable=usb*  -device usb-ehci,id=ehci -drive if=none,id=usbstick,file=../../myos.iso -device usb-storage,bus=ehci.0,drive=usbstick

#define EHCI_PERIODIC_FRAME_SIZE 1024

unsigned long periodic_list[EHCI_PERIODIC_FRAME_SIZE] __attribute__ ((aligned (0x1000)));
unsigned long portbaseaddress;
unsigned long usbcmd_addr;
unsigned long usbsts_addr;
unsigned long usbint_addr;
unsigned long usbfin_addr;
unsigned long usbctr_addr;
unsigned long usbper_addr;
unsigned long usbasc_addr;
unsigned long usbcon_addr;
unsigned char available_ports;

extern void ehciirq();
void irq_ehci(){
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
    
    //
    // what is happening?
    unsigned long status = ((unsigned long*)usbsts_addr)[0];
    if(status&0b000001){
        printf("[EHCI] Interrupt: transaction completed\n");
    }
    if(status&0b000010){
        printf("[EHCI] Interrupt: error interrupt\n");
    }
    if(status&0b000100){
        printf("[EHCI] Interrupt: portchange\n");
    }
    if(status&0b001000){
        printf("[EHCI] Interrupt: frame list rollover\n");
    }
    if(status&0b010000){
        printf("[EHCI] Interrupt: host system error\n");
    }
    if(status&0b100000){
        printf("[EHCI] Interrupt: interrupt on advance\n");
    }
    ((unsigned long*)usbsts_addr)[0] = status;
}

void ehci_add_command(unsigned long instructioncommand){

    //
    // add value to first available point
    int index = 0;
    for(int i = 0 ; i < EHCI_PERIODIC_FRAME_SIZE ; i++){
        if(periodic_list[i]&1){
            index = i;
        }
    }
    printf("[EHCI] At index %x \n",index);

    //
    // setup basic token
    unsigned long message = (instructioncommand & 0xFFFFFFE0);
    periodic_list[index] = message;
}

void init_ehci_port(int portnumber){
        unsigned long avail_port_addr = portbaseaddress + 0x44 + (portnumber*4);
        unsigned long portinfo = ((unsigned long*)avail_port_addr)[0];

        // reset the port
        ((unsigned long*)avail_port_addr)[0] |=  0b100000000;
        sleep(20);
        ((unsigned long*)avail_port_addr)[0] &= ~0b100000000;
        sleep(20);
        portinfo = ((volatile unsigned long*)avail_port_addr)[0];
        printf("[EHCI] Port %x : End of port reset with %x \n",portnumber,portinfo);

        // check if port is enabled
        if(portinfo&0b100){
            printf("[EHCI] Port %x : Port is enabled\n",portnumber);
        }else{
            printf("[EHCI] Port %x : Port is not enabled but connected\n",portnumber);
            return;
        }

        // activating
        
}

void ehci_probe(){
    // checking all ports
    for(int i = 0 ; i < available_ports ; i++){
        unsigned long avail_port_addr = portbaseaddress + 0x44 + (i*4);
        unsigned long portinfo = ((unsigned long*)avail_port_addr)[0];
        printf("[EHCI] Port %x info : %x \n",i,portinfo);
        if(portinfo&2){
            printf("[EHCI] Port %x : connection detected!\n",i);
            init_ehci_port(i);
        }
    }
}

void init_ehci(unsigned long bus,unsigned long slot,unsigned long function){
    printf("[EHCI] Entering EHCI module\n");
    unsigned long baseaddress = getBARaddress(bus,slot,function,0x10);
    printf("[EHCI] The base address of EHCI is %x \n",baseaddress);
    portbaseaddress = baseaddress + ((unsigned char*)baseaddress)[0];
    printf("[EHCI] The address to the first EHCI registers is %x \n",portbaseaddress);
    unsigned long hci_version_addr = baseaddress+0x02;
    unsigned short hci_version = ((unsigned short*)hci_version_addr)[0];
    printf("[EHCI] The versionnumber of the EHCI protocol is %x \n",hci_version);
    if(hci_version!=0x100){
        printf("[EHCI] This driver is not compliant with current EHCIprotocol version\n");
        return;
    }
    unsigned long hcsparams_addr = baseaddress+0x04;
    unsigned long hcsparams = ((unsigned long*)hcsparams_addr)[0];
    printf("[EHCI] HCSParams are %x \n",hcsparams);
    available_ports = hcsparams & 0b1111;
    printf("[EHCI] We have %x ports available\n",available_ports);
    unsigned long hccparams_addr = baseaddress+0x08;
    unsigned long hccparams = ((unsigned long*)hccparams_addr)[0];
    printf("[EHCI] HCCParams are %x \n",hccparams);
    unsigned char bit64cap = hccparams & 1;
    if(bit64cap){
        printf("[EHCI] This controller is 64 bit capable\n");
    }
    unsigned char capabilitypointer = (hccparams & 0b1111111100000000) >> 8;
    if(capabilitypointer){
        unsigned long capabilitypointer_addr =  capabilitypointer;
        printf("[EHCI] We have capabilitypointers at %x \n",capabilitypointer_addr);
        while(1){
            unsigned long cap = getBARaddress(bus,slot,function,capabilitypointer_addr);
            unsigned char cid = cap & 0xFF;
            if(cid==0x01){
                printf("[EHCI] Legacy support available\n");
                if(cap&(1<<16)){
                    printf("[EHCI] BIOS owns controller\n");
                }
            }else{
                printf("[EHCI] Unknown extended cappoint %x : %x \n",cid,cap);
            } 
            unsigned char cxt = (cap & 0xFF00)>>8;
            if(cxt==0){
                break;
            }
            capabilitypointer_addr += cxt;
        }
    }
	unsigned long usbint = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
    setNormalInt(usbint,(unsigned long)ehciirq);
    printf("[EHCI] Installing interrupts at %x \n",usbint);

    //
    // Now, let the hostcontroller do what we are asking.
    usbcmd_addr = portbaseaddress + 0x00;
    usbsts_addr = portbaseaddress + 0x04;
    usbint_addr = portbaseaddress + 0x08;
    usbfin_addr = portbaseaddress + 0x0C;
    usbctr_addr = portbaseaddress + 0x10;
    usbper_addr = portbaseaddress + 0x14;
    usbasc_addr = portbaseaddress + 0x18;
    usbcon_addr = portbaseaddress + 0x40;

    // stop already running controllers
    unsigned long default_usbcmd_value = ((unsigned long*)usbcmd_addr)[0];
    printf("[EHCI] Default value USBCMD %x \n",default_usbcmd_value);
    if(default_usbcmd_value & 1){
        printf("[EHCI] EHCI is already running. Stopping it.\n");
        ((unsigned long*)usbcmd_addr)[0] &= ~1; // stopping
        while(1){
            if((((volatile unsigned long*)usbcmd_addr)[0]&1)==0){
                break;
            }
        }
        // stop succesfull
    }

    // Reset the controller
    ((unsigned long*)usbcmd_addr)[0] |= 2;
    while(1){
        if((((volatile unsigned long*)usbcmd_addr)[0]&2)==0){
            break;
        }
    }
    printf("[EHCI] Reset of EHCI Host Controller is succeed.\n");

    // clearning periodic_list
    for(int i = 0 ; i < EHCI_PERIODIC_FRAME_SIZE ; i++){
        periodic_list[i] |= 1;
    }

    // set correct segment
    ((unsigned long*)usbctr_addr)[0] = 0;

    // set correct ints
    ((unsigned long*)usbint_addr)[0] |= 0b10111; // enable all interrupts except: frame_list_rollover async_adv

    // set periodic list base
    ((unsigned long*)usbper_addr)[0] = (unsigned long)&periodic_list;

    // set interrupt treshold (usbcmd) : default is 8mf with value 08
    ((unsigned long*)usbcmd_addr)[0] |= (0x40<<16);

    // set frame list size : default is 1024 with value 0
    ((unsigned long*)usbcmd_addr)[0] |= (0<<2);

    // set run bit
    ((unsigned long*)usbcmd_addr)[0] |= 1;

    // set routing
    ((unsigned long*)usbcon_addr)[0] |= 1;

    ehci_probe();

    for(;;);
}