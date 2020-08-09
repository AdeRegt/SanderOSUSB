#include "../kernel.h"
// OSDEV article: https://wiki.osdev.org/USB_Human_Interface_Devices

unsigned char usb_get_packed_hid(USB_DEVICE* device,unsigned long devicedesc){
	EhciCMD* commando = (EhciCMD*) malloc(sizeof(EhciCMD));
	commando->bRequest = 1;
	commando->bRequestType = 0xA1;
    commando->wIndex = 0;
    commando->wLength = 3;
    commando->wValue = 0x0100;
	unsigned char *res = usb_send_and_recieve_control(device,commando,devicedesc);
	free(commando);
	return res;
}

unsigned char get_usb_hid_keyboard_input(USB_DEVICE* device,unsigned char wait){
	unsigned volatile char devicedesc[10];
	again:
	wait++;
	unsigned char res = usb_get_packed_hid(device,(unsigned long)&devicedesc);
	if(res!=EHCI_ERROR){
		if(devicedesc[2]!=0){
			unsigned char tx = devicedesc[2];
			if(tx<0xFF){
				unsigned char kbdus[128] ={
				1,
				1,
				1,
				1,
				'a', // 0x04
				'b',
				'c',
				'd',
				'e', // 0x08
				'f',
				'g',
				'h',
				'i', // 0x0C
				'j',
				'k',
				'l',
				'm',
				'n',
				'o', // 0x12
				'p', // 0x13
				'q', // 0x14
				'r', // 0x15
				's',
				't', // 0x17
				'u', // 0x18
				'v',
				'w', // 0x1A
				'x',
				'y', // 0x1C
				'z',
				'1',
				'2',
				'3',
				'4',
				'5',
				'6',
				'7',
				'8',
				'9'
				};
				if(tx==0x52){
					return VK_UP;
				}else if(tx==0x51){
					return VK_DOWN;
				}else if(tx==0x50){
					return VK_LEFT;
				}else if(tx==0x4F){
					return VK_RIGHT;
				}else if(tx==0x28){
					return '\n';
				}else{
					return kbdus[tx];
				}
			}
		}
	}else{
		return 0;
	}
	if(wait){
		sleep(100);
		goto again;
	}
	return 0;
}

//void init_usb_hid_mouse(USB_DEVICE* device){
//	unsigned char (*send)(USB_DEVICE* device,TRB setup,TRB data,TRB end) = (void*)device->sendMessage;
//}

unsigned long usb_hid_has_keyboard = 0;

unsigned long usb_get_keyboard(){
	return usb_hid_has_keyboard;
}

void init_usb_hid_keyboard(USB_DEVICE* device){
	//
	// Use GET_REPORT protocol
	usb_hid_has_keyboard = (unsigned long)device;
	
	return;
}

void init_usb_hid(USB_DEVICE* device){
	
	printf("[HID] Port %x : Is a Human Interface Device\n",device->portnumber);
	if(device->subclass==1){
		printf("[HID] Port %x : HID : bootprotocol supported\n",device->portnumber);
	}else if(device->subclass==0){
		printf("[HID] Port %x : HID : reportprotocol supported\n",device->portnumber);
	}else{
		printf("[HID] Port %x : HID : unknown protocol\n",device->portnumber);
	}
	
	//
	// Use SET_PROTOCOL to boot protocol
	EhciCMD* commando = (EhciCMD*) malloc(sizeof(EhciCMD));
	commando->bRequest = 0x0B;
	commando->bRequestType = 0x21;
    commando->wIndex = 0;
    commando->wLength = 0;
    commando->wValue = 0;
	unsigned char *res = usb_send_and_recieve_control(device,commando,0);
	free(commando);
	if((unsigned long)res==EHCI_ERROR){
		printf("[HID] Unable to set protocol\n");
		return;
	}

	//
	// Init the rest of the devices
	if(device->protocol==1){
		printf("[HID] Port %x : is keyboard\n",device->portnumber);
		init_usb_hid_keyboard(device);
	}else if(device->protocol==2){
		printf("[HID] Port %x : is mouse\n",device->portnumber);
	}else{
		printf("[HID] Port %x : unknown HID type %x \n",device->portnumber,device->protocol);
	}
}
