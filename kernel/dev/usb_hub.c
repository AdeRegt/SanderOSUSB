#include "../kernel.h"

void usb_hub_init(USB_DEVICE *device){
    printf("[HUB] %x %x %x\n",device->class,device->subclass,device->protocol);
    for(;;);
}