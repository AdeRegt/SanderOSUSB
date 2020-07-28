#include "../kernel.h"
// cd ../kernel
// bash build.sh 
// ../../qemu/x86_64-softmmu/qemu-system-x86_64 --cdrom ../cdrom.iso  -trace enable=usb*  -device usb-ehci,id=ehci -drive if=none,id=usbstick,file=../../myos.iso -device usb-storage,bus=ehci.0,drive=usbstick
// much thanks to https://github.com/dgibson/SLOF and https://github.com/pdoane/osdev and https://github.com/coreboot/seabios

#define EHCI_PERIODIC_FRAME_SIZE    1024

typedef struct {
    volatile unsigned long nextlink;
    volatile unsigned long altlink;
    volatile unsigned long token;
    volatile unsigned long buffer[5];
    volatile unsigned long extbuffer[5];
}EhciTD;

typedef struct {
    volatile unsigned long horizontal_link_pointer;
    volatile unsigned long characteristics;
    volatile unsigned long capabilities;
    volatile unsigned long curlink;

    volatile unsigned long nextlink;
    volatile unsigned long altlink;
    volatile unsigned long token;
    volatile unsigned long buffer[5];
    volatile unsigned long extbuffer[5];
    
}EhciQH;

struct usb_config_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  bMaxPower;
} __attribute__ ((packed));

struct usb_interface_descriptor {
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned char  bInterfaceNumber;
    unsigned char  bAlternateSetting;
    unsigned char  bNumEndpoints;
    unsigned char  bInterfaceClass;
    unsigned char  bInterfaceSubClass;
    unsigned char  bInterfaceProtocol;
    unsigned char  iInterface;
} __attribute__ ((packed));


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
    }else if(status&0b000010){
        printf("[EHCI] Interrupt: error interrupt\n");
    }else if(status&0b000100){
        printf("[EHCI] Interrupt: portchange\n");
    }else if(status&0b001000){
        printf("[EHCI] Interrupt: frame list rollover\n");
    }else if(status&0b010000){
        printf("[EHCI] Interrupt: host system error\n");
    }else if(status&0b100000){
        printf("[EHCI] Interrupt: interrupt on advance\n");
    }else{
        printf("[EHCI] Interrupt: unknown\n");
    }
    ((unsigned long*)usbsts_addr)[0] = status;
}


unsigned char ehci_wait_for_completion(volatile EhciTD *status){
    unsigned char lstatus = 1;
    resetTicks();
    while(1){
        volatile unsigned long tstatus = (volatile unsigned long)status->token;
        if(tstatus & (1 << 4)){
            // not anymore active and failed miserably
            printf("[EHCI] Transmission failed due babble error\n");
            lstatus = 0;
            break;
        }
        if(tstatus & (1 << 3)){
            // not anymore active and failed miserably
            printf("[EHCI] Transmission failed due transaction error\n");
            lstatus = 0;
            break;
        }
        if(tstatus & (1 << 6)){
            // not anymore active and failed miserably
            printf("[EHCI] Transmission failed due serious error\n");
            lstatus = 0;
            break;
        }
        if(tstatus & (1 << 5)){
            // not anymore active and failed miserably
            printf("[EHCI] Transmission failed due data buffer error\n");
            lstatus = 0;
            break;
        }
        if(!(tstatus & (1 << 7))){
            // not anymore active and succesfull ended
            lstatus = 1;
            break;
        }
        if(getTicks()>50){
            printf("EHCI FAIL Timeout\n");
            lstatus = 0;
            break;
        }
    }
    if(lstatus==0){
        //
        // Something is wrong... can we discover what is wrong?
        unsigned char errorcount = (status->token >> 10) & 0b111; // maxerror
        // According to the specification of ehci, for CRC, Timeout,Bad PID the result is 0
        // For Babble and buffererror , result is not 0
    }
    return lstatus;
}

unsigned char ehci_set_device_address(unsigned char addr){

    //
    // ok, to set a address to a device, we need 4 components:
    // 1) QH: a queuehead to connect to us
    // 2) QTD: a command descriptor
    // 3) QTD: a status descriptor
    // 4) CMD: a command

    // first make the command
    EhciCMD *cmd = (EhciCMD*) malloc_align(sizeof(EhciCMD),0xFFF);
    cmd->bRequest = 0x05; // USB-REQUEST type SET-ADDR is 0x05
    cmd->bRequestType |= 0; // direction is out
    cmd->bRequestType |= 0 << 5; // type is standard
    cmd->bRequestType |= 0; // recieve
    cmd->wIndex = 0; // ok
    cmd->wLength = 0; // ok
    cmd->wValue = addr;

    EhciTD *command = (EhciTD*) malloc_align(sizeof(EhciTD),0xFF);
    volatile EhciTD *status = (volatile EhciTD*) malloc_align(sizeof(EhciTD),0xFF);
    EhciQH *head1 = (EhciQH*) malloc_align(sizeof(EhciQH),0xFF);
    EhciQH *head2 = (EhciQH*) malloc_align(sizeof(EhciQH),0xFF);

    command->nextlink = (unsigned long)status;
    command->altlink = 1;
    command->token |= (8 << 16); // setup size
    command->token |= (1 << 7); // actief
    command->token |= (0x2 << 8); // type is setup
    command->token |= (0x3 << 10); // maxerror
    command->buffer[0] = (unsigned long)cmd;

    status->nextlink = 1;
    status->altlink = 1;
    status->token |= (1 << 8); // PID instellen
    status->token |= (1 << 31); // toggle
    status->token |= (1 << 7); // actief
    status->token |= (0x3 << 10); // maxerror

    //
    // Tweede commando


    head2->altlink = 1;
    head2->nextlink = (unsigned long)command; // qdts2
    head2->horizontal_link_pointer = ((unsigned long)head1) | 2;
    head2->curlink = 0; // qdts1
    head2->characteristics |= 1 << 14; // dtc
    head2->characteristics |= 64 << 16; // mplen
    head2->characteristics |= 2 << 12; // eps
    head2->capabilities = 0x40000000;

    //
    // Eerste commando
    head1->altlink = 1;
    head1->nextlink = 1;
    head1->horizontal_link_pointer = ((unsigned long)head2) | 2;
    head1->curlink = 0;
    head1->characteristics = 1 << 15; // T
    head1->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)head1) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char lstatus = ehci_wait_for_completion(status);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    return lstatus;
}

unsigned char* ehci_get_device_descriptor(unsigned char addr,unsigned char size){
    EhciQH* qh = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh2 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh3 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciTD* td = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    EhciTD* trans = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    volatile EhciTD* status = (volatile EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);

    unsigned char *buffer = malloc(size);

    commando->bRequest = 0x06; // get_descriptor
    commando->bRequestType |= 0x80; // dir=IN
    commando->wIndex = 0; // windex=0
    commando->wLength = size; // getlength=8
    commando->wValue = 1 << 8; // get device info

    //
    // Derde commando
    td->token |= 2 << 8; // PID=2
    td->token |= 3 << 10; // CERR=3
    td->token |= size << 16; // TBYTE=8
    td->token |= 1 << 7; // ACTIVE=1
    td->nextlink = (unsigned long)trans;
    td->altlink = 1;
    td->buffer[0] = (unsigned long)commando;

    trans->nextlink = (unsigned long)status;
    trans->altlink = 1;
    trans->token |= (size << 16); // verwachte lengte
    trans->token |= (1 << 31); // toggle
    trans->token |= (1 << 7); // actief
    trans->token |= (1 << 8); // IN token
    trans->token |= (0x3 << 10); // maxerror
    trans->buffer[0] = (unsigned long)buffer;

    status->nextlink = 1;
    status->altlink = 1;
    status->token |= (0 << 8); // PID instellen
    status->token |= (1 << 31); // toggle
    status->token |= (1 << 7); // actief
    status->token |= (0x3 << 10); // maxerror


    //
    // Tweede commando
    qh2->altlink = 1;
    qh2->nextlink = (unsigned long)td; // qdts2
    qh2->horizontal_link_pointer = ((unsigned long)qh) | 2;
    qh2->curlink = (unsigned long)qh3; // qdts1
    qh2->characteristics |= 1 << 14; // dtc
    qh2->characteristics |= 64 << 16; // mplen
    qh2->characteristics |= 2 << 12; // eps
    qh2->characteristics |= addr; // device
    qh2->capabilities = 0x40000000;

    //
    // Eerste commando
    qh->altlink = 1;
    qh->nextlink = 1;
    qh->horizontal_link_pointer = ((unsigned long)qh2) | 2;
    qh->curlink = 0;
    qh->characteristics = 1 << 15; // T
    qh->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)qh) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char result = ehci_wait_for_completion(status);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    if(result==0){
        return 0;
    }

    return buffer;
}

unsigned char* ehci_get_device_configuration(unsigned char addr,unsigned char size){
    EhciQH* qh = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh2 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh3 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciTD* td = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    EhciTD* trans1 = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    EhciTD* trans2 = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    volatile EhciTD* status = (volatile EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);

    unsigned char *buffer = malloc(size);

    commando->bRequest = 0x06; // get_descriptor
    commando->bRequestType |= 0x80; // dir=IN
    commando->wIndex = 0; // windex=0
    commando->wLength = size; // getlength=8
    commando->wValue = 2 << 8; // get config info

    //
    // Derde commando
    td->token |= 2 << 8; // PID=2
    td->token |= 3 << 10; // CERR=3
    td->token |= 8 << 16; // TBYTE=8
    td->token |= 1 << 7; // ACTIVE=1
    td->nextlink = (unsigned long)trans1;
    td->altlink = 1;
    td->buffer[0] = (unsigned long)commando;
    unsigned char toggle = 0;
    toggle ^= 1;

    trans1->nextlink = (unsigned long)status;//trans2;
    trans1->altlink = 1;
    trans1->token |= (size << 16); // verwachte lengte
    trans1->token |= (toggle << 31); // toggle
    trans1->token |= (1 << 7); // actief
    trans1->token |= (1 << 8); // IN token
    trans1->token |= (0x3 << 10); // maxerror
    trans1->buffer[0] = (unsigned long)buffer;

    toggle ^= 1;

    trans2->nextlink = (unsigned long)status;
    trans2->altlink = 1;
    trans2->token |= (size << 16); // verwachte lengte
    trans2->token |= (toggle << 31); // toggle
    trans2->token |= (1 << 7); // actief
    trans2->token |= (1 << 8); // IN token
    trans2->token |= (0x3 << 10); // maxerror
    trans2->buffer[0] = ((unsigned long)buffer)+8;

    status->nextlink = 1;
    status->altlink = 1;
    status->token |= (0 << 8); // PID instellen
    status->token |= (1 << 31); // toggle
    status->token |= (1 << 7); // actief
    status->token |= (0x3 << 10); // maxerror


    //
    // Tweede commando
    qh2->altlink = 1;
    qh2->nextlink = (unsigned long)td; // qdts2
    qh2->horizontal_link_pointer = ((unsigned long)qh) | 2;
    qh2->curlink = (unsigned long)qh3; // qdts1
    qh2->characteristics |= 1 << 14; // dtc
    qh2->characteristics |= 64 << 16; // mplen
    qh2->characteristics |= 2 << 12; // eps
    qh2->characteristics |= addr; // device
    qh2->capabilities = 0x40000000;

    //
    // Eerste commando
    qh->altlink = 1;
    qh->nextlink = 1;
    qh->horizontal_link_pointer = ((unsigned long)qh2) | 2;
    qh->curlink = 0;
    qh->characteristics = 1 << 15; // T
    qh->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)qh) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char result = ehci_wait_for_completion(status);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    if(result==0){
        return 0;
    }

    return buffer;
}

unsigned char ehci_set_device_configuration(unsigned char addr,unsigned char config){

    //
    // ok, to set a address to a device, we need 4 components:
    // 1) QH: a queuehead to connect to us
    // 2) QTD: a command descriptor
    // 3) QTD: a status descriptor
    // 4) CMD: a command

    // first make the command
    EhciCMD *cmd = (EhciCMD*) malloc_align(sizeof(EhciCMD),0xFFF);
    cmd->bRequest = 0x09; // USB-REQUEST type SET-CONFIGURATION is 0x09
    cmd->bRequestType |= 0; // direction is out
    cmd->bRequestType |= 0 << 5; // type is standard
    cmd->bRequestType |= 0; // recieve
    cmd->wIndex = 0; // ok
    cmd->wLength = 0; // ok
    cmd->wValue = config;

    EhciTD *command = (EhciTD*) malloc_align(sizeof(EhciTD),0xFF);
    volatile EhciTD *status = (volatile EhciTD*) malloc_align(sizeof(EhciTD),0xFF);
    EhciQH *head1 = (EhciQH*) malloc_align(sizeof(EhciQH),0xFF);
    EhciQH *head2 = (EhciQH*) malloc_align(sizeof(EhciQH),0xFF);

    command->nextlink = (unsigned long)status;
    command->altlink = 1;
    command->token |= (8 << 16); // setup size
    command->token |= (1 << 7); // actief
    command->token |= (0x2 << 8); // type is setup
    command->token |= (0x3 << 10); // maxerror
    command->buffer[0] = (unsigned long)cmd;

    status->nextlink = 1;
    status->altlink = 1;
    status->token |= (1 << 8); // PID instellen
    status->token |= (1 << 31); // toggle
    status->token |= (1 << 7); // actief
    status->token |= (0x3 << 10); // maxerror

    //
    // Tweede commando


    head2->altlink = 1;
    head2->nextlink = (unsigned long)command; // qdts2
    head2->horizontal_link_pointer = ((unsigned long)head1) | 2;
    head2->curlink = 0; // qdts1
    head2->characteristics |= 1 << 14; // dtc
    head2->characteristics |= 64 << 16; // mplen
    head2->characteristics |= 2 << 12; // eps
    head2->characteristics |= addr; // device
    head2->capabilities = 0x40000000;

    //
    // Eerste commando
    head1->altlink = 1;
    head1->nextlink = 1;
    head1->horizontal_link_pointer = ((unsigned long)head2) | 2;
    head1->curlink = 0;
    head1->characteristics = 1 << 15; // T
    head1->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)head1) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char lstatus = ehci_wait_for_completion(status);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    return lstatus;
}

unsigned char* ehci_send_and_recieve_command(unsigned char addr,EhciCMD* commando){
    EhciQH* qh = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh2 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh3 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciTD* td = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    EhciTD* trans = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
    volatile EhciTD* status = (volatile EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);

    unsigned char *buffer = malloc(commando->wLength);

    //
    // Derde commando
    td->token |= 2 << 8; // PID=2
    td->token |= 3 << 10; // CERR=3
    td->token |= 8 << 16; // TBYTE=8
    td->token |= 1 << 7; // ACTIVE=1
    td->nextlink = (unsigned long)trans;
    td->altlink = 1;
    td->buffer[0] = (unsigned long)commando;

    trans->nextlink = (unsigned long)status;
    trans->altlink = 1;
    trans->token |= (commando->wLength << 16); // verwachte lengte
    trans->token |= (1 << 31); // toggle
    trans->token |= (1 << 7); // actief
    trans->token |= (1 << 8); // IN token
    trans->token |= (0x3 << 10); // maxerror
    trans->buffer[0] = (unsigned long)buffer;

    status->nextlink = 1;
    status->altlink = 1;
    status->token |= (0 << 8); // PID instellen
    status->token |= (1 << 31); // toggle
    status->token |= (1 << 7); // actief
    status->token |= (0x3 << 10); // maxerror


    //
    // Tweede commando
    qh2->altlink = 1;
    qh2->nextlink = (unsigned long)td; // qdts2
    qh2->horizontal_link_pointer = ((unsigned long)qh) | 2;
    qh2->curlink = (unsigned long)qh3; // qdts1
    qh2->characteristics |= 1 << 14; // dtc
    qh2->characteristics |= 64 << 16; // mplen
    qh2->characteristics |= 2 << 12; // eps
    qh2->characteristics |= addr; // device
    qh2->capabilities = 0x40000000;

    //
    // Eerste commando
    qh->altlink = 1;
    qh->nextlink = 1;
    qh->horizontal_link_pointer = ((unsigned long)qh2) | 2;
    qh->curlink = 0;
    qh->characteristics = 1 << 15; // T
    qh->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)qh) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char result = ehci_wait_for_completion(status);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    if(result==0){
        return (unsigned char*)EHCI_ERROR;
    }

    return buffer;
}

unsigned long ehci_send_bulk(unsigned char addr,unsigned char* out,unsigned long expectedOut,unsigned char in1){
    //
    // Send bulk
    EhciTD *command = (EhciTD*) malloc_align(sizeof(EhciTD),0xFF);
    EhciQH *head1 = (EhciQH*) malloc_align(sizeof(EhciQH),0xFF);
    EhciQH *head2 = (EhciQH*) malloc_align(sizeof(EhciQH),0xFF);

    command->nextlink = 1;
    command->altlink = 1;
    command->token |= (expectedOut << 16); // setup size
    command->token |= (1 << 7); // actief
    command->token |= (0 << 8); // type is out
    command->token |= (0x3 << 10); // maxerror
    command->buffer[0] = (unsigned long)out;

    //
    // Tweede commando


    head2->altlink = 1;
    head2->nextlink = (unsigned long)command; // qdts2
    head2->horizontal_link_pointer = ((unsigned long)head1) | 2;
    head2->curlink = 0; // qdts1
    head2->characteristics |= 512 << 16; // mplen
    head2->characteristics |= 2 << 12; // eps
    head2->characteristics |= addr; // device
    head2->characteristics |= (in1?2:1) << 8; // endpoint 2
    head2->capabilities = 0x40000000;
    //printf("[EHCI] BULK: Sending %x \n",head2->characteristics); //  heeft = 0x2006201 moet = 0x2002201

    //
    // Eerste commando
    head1->altlink = 1;
    head1->nextlink = 1;
    head1->horizontal_link_pointer = ((unsigned long)head2) | 2;
    head1->curlink = 0;
    head1->characteristics = 1 << 15; // T
    head1->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)head1) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char lstatus = ehci_wait_for_completion(command);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    if(lstatus==0){
        return (unsigned long)EHCI_ERROR;
    }
    return 0;
}

unsigned char* ehci_recieve_bulk(unsigned char addr,unsigned long expectedIN,unsigned char in1){
    //
    // Recieve bulk
    //printf("[EHCI] BULK: Recieving\n");
    EhciQH* qh = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciQH* qh2 = (EhciQH*) malloc_align(sizeof(EhciQH),0x1FF);
    EhciTD* trans = 0;
    EhciTD* trans1 = 0;

    unsigned char *buffer = malloc(expectedIN);

    // wacht totdat alle bytes zijn ingeladen, dit zijn er 144 per keer.
    unsigned long wachtend = 0;
    unsigned long bytesperkeer = in1?144:512; //144
    unsigned char reject = 0;
    unsigned char tx = 0;
    unsigned char forcestop = 0;
    while(1){
        if(trans==0){
            trans = (EhciTD*) malloc_align(sizeof(EhciTD),0x1FF);
            trans1 = trans;
        }else{
            unsigned long yo = (unsigned long)malloc_align(sizeof(EhciTD),0x1FF);
            trans->nextlink = yo;
            trans = (EhciTD*) yo;
        }
        //printf("pre :: wachtend=%x bytesperkeer=%x expectedin=%x \n",wachtend,bytesperkeer,expectedIN);
        trans->altlink = 1;
        if(bytesperkeer>expectedIN){
            trans->token |= (expectedIN << 16);
            forcestop = 1;
        }else if((wachtend+bytesperkeer)>expectedIN){
            reject = 1;
            unsigned long twt = expectedIN-wachtend;
            trans->token |= (twt << 16);
            // over = 40 | wachtend = 1b0 | optel = 1f0 | max = 200
            //printf("over=%x wachtend=%x optel=%x max=%x\n",twt,wachtend,wachtend+twt,expectedIN);
        }else{
            trans->token |= (bytesperkeer << 16); // verwachte lengte | expectedIN
            //printf("RB2=%x \n",bytesperkeer);
        }
        trans->token |= (1 << 31); // toggle
        trans->token |= (1 << 7); // actief
        trans->token |= (1 << 8); // IN token
        trans->token |= (0x3 << 10); // maxerror
        trans->buffer[0] = (unsigned long)buffer + (tx*bytesperkeer);
        wachtend += bytesperkeer;
        tx++;
        //printf("post :: wachtend=%x bytesperkeer=%x expectedin=%x \n",wachtend,bytesperkeer,expectedIN);
        if(wachtend>expectedIN&&reject){
            break;
        }else if(wachtend==expectedIN){
            break;
        }else if(forcestop==1){
            //printf("reached forcestop\n");
            break;
        }
    }
    trans->nextlink = 1;

    //
    // Tweede commando
    qh2->altlink = 1;
    qh2->nextlink = (unsigned long)trans1; // qdts2
    qh2->horizontal_link_pointer = ((unsigned long)qh) | 2;
    qh2->curlink = 1; // qdts1
    qh2->characteristics |= 1 << 14; // dtc
    qh2->characteristics |= 512 << 16; // mplen
    qh2->characteristics |= 2 << 12; // eps
    qh2->characteristics |= 1 << 8; // endpoint 1
    qh2->characteristics |= addr; // device
    qh2->capabilities = 0x40000000;

    //
    // Eerste commando
    qh->altlink = 1;
    qh->nextlink = 1;
    qh->horizontal_link_pointer = ((unsigned long)qh2) | 2;
    qh->curlink = 0;
    qh->characteristics = 1 << 15; // T
    qh->token = 0x40;

    ((unsigned long*) usbasc_addr)[0] = ((unsigned long)qh) ;
    ((unsigned long*)usbcmd_addr)[0] |= 0b100000;

    unsigned char result = ehci_wait_for_completion(trans);
    
    ((unsigned long*)usbcmd_addr)[0] &= ~0b100000;
    ((unsigned long*) usbasc_addr)[0] = 1 ;
    if(result==0){
        for(;;);
        return (unsigned char *)EHCI_ERROR;
    }

    return buffer;
}

unsigned char* ehci_send_and_recieve_bulk(unsigned char addr,unsigned char* out,unsigned long expectedIN,unsigned long expectedOut,unsigned char in1){


    unsigned long lstatus = ehci_send_bulk(addr,out,expectedOut,in1);
    if(lstatus==EHCI_ERROR){
        return (unsigned char*)EHCI_ERROR;
    }

    unsigned char* buffer = ehci_recieve_bulk(addr,expectedIN,in1);
    if((unsigned long)buffer==EHCI_ERROR){
        return (unsigned char *)EHCI_ERROR;
    }

    return buffer;
}

void init_ehci_port(int portnumber){
    unsigned long avail_port_addr = portbaseaddress + 0x44 + (portnumber*4);
    unsigned long portinfo = ((unsigned long*)avail_port_addr)[0];

    // reset the port
    ((unsigned long*)avail_port_addr)[0] |=  0b100000000;
    sleep(60);
    ((unsigned long*)avail_port_addr)[0] &= ~0b100000000;
    sleep(20);
    portinfo = ((volatile unsigned long*)avail_port_addr)[0];
    printf("[EHCI] Port %x : End of port reset with %x \n",portnumber,portinfo);

    // check if port is enabled
    if((portinfo&0b100)==0){
        printf("[EHCI] Port %x : Port is not enabled but connected\n",portnumber);
        return;
    }

    // set device address to device
    unsigned char deviceaddress = 1;
    printf("[EHCI] Port %x : Setting device address to %x \n",portnumber,deviceaddress);
    unsigned char deviceaddressok = ehci_set_device_address(deviceaddress);
    if(deviceaddressok==0){
        printf("[EHCI] Port %x : Unable to set device address...\n",portnumber);
        return;
    }

    // get device descriptor
    printf("[EHCI] Port %x : Getting devicedescriptor...\n",portnumber);
    unsigned char* desc = ehci_get_device_descriptor(deviceaddress,8);//8
    if(desc==0){
        printf("[EHCI] Port %x : Unable to get devicedescriptor...\n",portnumber);
        return;
    }

    // check result
    if(!(desc[0]==0x12&&desc[1]==0x1)){
        printf("[EHCI] Port %x : Invalid magic number at descriptor (%x %x)...\n",portnumber,desc[0],desc[1]);
        return;
    }
    unsigned char maxpacketsize = desc[7];
    printf("[EHCI] Port %x : Max Packet Size %x \n",portnumber,maxpacketsize);

    unsigned char deviceclass = desc[4];
    unsigned char deviceSubClass = 0;
    unsigned char deviceProtocol = 0;
    if(deviceclass==0x00){
        printf("[ECHI] Port %x : No class found! Asking descriptors...\n",portnumber);
        struct usb_config_descriptor* sec = (struct usb_config_descriptor*)ehci_get_device_configuration(deviceaddress,sizeof(struct usb_interface_descriptor) + sizeof(struct usb_config_descriptor));
        if(sec==0){
            printf("[ECHI] Port %x : Failed to read descriptors...\n",portnumber);
            return;
        }
        struct usb_interface_descriptor* desc = (struct usb_interface_descriptor*)(((unsigned long)sec)+9);
        deviceclass = desc->bInterfaceClass;
        deviceSubClass = desc->bInterfaceSubClass;
        deviceProtocol = desc->bInterfaceProtocol;
        printf("[EHCI] Port %x : There are %x endpoints available\n",portnumber,desc->bNumEndpoints);
    }

    if(deviceclass==0x00){
        printf("[EHCI] Port %x : Still unable to get device class\n",portnumber);
        return;
    }
    printf("[EHCI] Port %x : We have a deviceclass. Device class is %x \n",portnumber,deviceclass);

    printf("[EHCI] Port %x : Set default configuration for device\n",portnumber);
    unsigned char setdevconfigresult = ehci_set_device_configuration(deviceaddress,1);
    if(setdevconfigresult==0){
        printf("[EHCI] Port %x : Unable to set configuration\n",portnumber);
        return;
    }
    if(deviceclass==8){
        printf("[EHCI] Port %x : Mass Storage Device detected!\n",portnumber);
        ehci_stick_init(deviceaddress,deviceSubClass,deviceProtocol);
    }else if(deviceclass==3){
        printf("[EHCI] Port %x : Human Interface Device detected!\n",portnumber);
    }else{
        printf("[EHCI] Port %x : Unable to understand deviceclass: %x \n",portnumber,deviceclass);
    }
}

void ehci_probe(){
    // checking all ports
    for(int i = 0 ; i < available_ports ; i++){
        unsigned long avail_port_addr = portbaseaddress + 0x44 + (i*4);
        unsigned long portinfo = ((unsigned long*)avail_port_addr)[0];
        printf("[EHCI] Port %x info : %x \n",i,portinfo);
        if(portinfo&3){
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

}