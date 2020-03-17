#include "../kernel.h"

void init_xhci_hid(USB_DEVICE* device){
	printf("[XHCI@HID] Port %x : Is a Human Interface Device\n",device->portnumber);
	if(device->subclass==1){
		printf("[XHCI@HID] Port %x : HID : bootprotocol supported\n",device->portnumber);
	}else if(device->subclass==0){
		printf("[XHCI@HID] Port %x : HID : reportprotocol supported\n",device->portnumber);
	}else{
		printf("[XHCI@HID] Port %x : HID : unknown protocol\n",device->portnumber);
	}
	if(device->protocol==1){
		printf("[XHCI@HID] Port %x : is keyboard\n",device->portnumber);
	}else if(device->protocol==2){
		printf("[XHCI@HID] Port %x : is mouse\n",device->portnumber);
	}else{
		printf("[XHCI@HID] Port %x : unknown HID type %x \n",device->portnumber,device->protocol);
	}
	
}
