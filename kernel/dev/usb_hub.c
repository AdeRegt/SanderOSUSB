#include "../kernel.h"

struct USBHubDetails{
    unsigned char bDescLength;
    unsigned char bDescriptorType;
    unsigned char bNbrPorts;
    unsigned short wHubCharacteristics;
    unsigned char bPwrOn2PwrGood;
    unsigned char bHubContrCurrent;
} __attribute__ ((packed));

struct USBHubDetails *getHubDescriptors(USB_DEVICE *device, unsigned short type){
    struct USBHubDetails *uhd = (struct USBHubDetails*) malloc(sizeof(struct USBHubDetails));
    EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);
	commando->bRequest = 0xA0; // GET_HUB_DESCRIPTOR
    commando->bRequestType = 0x06; // GET_DESCRIPTOR
    commando->wIndex = 0; // windex=0
    commando->wLength = sizeof(struct USBHubDetails);
    commando->wValue = type; // get config info
	unsigned char *res = usb_send_and_recieve_control(device,commando,uhd);
	if((unsigned long)res==EHCI_ERROR){
        printf("[HUB] Failed to get hubdescriptor\n");
        return (struct USBHubDetails *)EHCI_ERROR;
    }
    return uhd;
}

unsigned char *getHubPortStatus(USB_DEVICE *device,unsigned char port){
    unsigned char *uhd = (unsigned char*) malloc(4);
    EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);
	commando->bRequest = 0xA3; // GET_PORT_STATUS
    commando->bRequestType = 0x00; // GET_STATUS
    commando->wIndex = port;
    commando->wLength = 4;
    commando->wValue = 0; // get config info
	unsigned char *res = usb_send_and_recieve_control(device,commando,uhd);
	if((unsigned long)res==EHCI_ERROR){
        printf("[HUB] Failed to get hubportstatus\n");
        return (struct USBHubDetails *)EHCI_ERROR;
    }
    return uhd;
}

void usb_hub_init(USB_DEVICE *device){
    printf("[HUB] %x %x %x\n",device->class,device->subclass,device->protocol);

    //
    // getting a hubdescriptor from the port....
    struct USBHubDetails *uhd = getHubDescriptors(device,0);
	if((unsigned long)uhd==EHCI_ERROR){
        printf("[HUB] Failed to get hubdescriptor\n");
        for(;;);
    }
    printf("[HUB] dDescLength=%x bDescriptorType=%x bNbrPorts=%x wHubCharacteristics=%x bPwrOn2PwrGood=%x bHubContrCurrent=%x \n",
    uhd->bDescLength,uhd->bDescriptorType,uhd->bNbrPorts,uhd->bPwrOn2PwrGood,uhd->wHubCharacteristics);
    for(unsigned char i = 1 ; i < uhd->bNbrPorts ; i++){
        unsigned char *tv = getHubPortStatus(device,i);
        printf("[HUB:%x] ARG1=%x ARG2=%x ARG3=%x ARG4=%x \n",tv[0],tv[1],tv[2],tv[3]);
    }
    for(;;);
}