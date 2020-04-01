#include "../kernel.h"
// OSDEV article: https://wiki.osdev.org/USB_Human_Interface_Devices

unsigned char xhci_get_packed_hid(USB_DEVICE* device,unsigned long devicedesc){
	unsigned char (*send)(USB_DEVICE* device,TRB setup,TRB data,TRB end) = (void*)device->sendMessage;
	
	TRB dc1;
	dc1.bar1 =  (0x0100<<16) | (1<<8) | 0xA1;//0x10900; 
	dc1.bar2 = (8<<16) | 0;
	dc1.bar3 = 0x8;
	dc1.bar4 = 0x841;
	
	TRB dc2;
	dc2.bar1 = devicedesc;
	dc2.bar2 = 0;
	dc2.bar3 = 8;
	dc2.bar4 = 0x10C01;
	
	TRB dc3;
	dc3.bar1 = 0;
	dc3.bar2 = 0;
	dc3.bar3 = 0;
	dc3.bar4 = 1 | (4<<10) | 0x20 | (1 << 16);
	
	unsigned char res = send(device,dc1,dc2,dc3);
	return res;
}

unsigned char get_xhci_hid_keyboard_input(USB_DEVICE* device,unsigned char wait){
	unsigned volatile char devicedesc[10];
	again:
	wait++;
	unsigned char res = xhci_get_packed_hid(device,(unsigned long)&devicedesc);
	if(res==1){
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

//void init_xhci_hid_mouse(USB_DEVICE* device){
//	unsigned char (*send)(USB_DEVICE* device,TRB setup,TRB data,TRB end) = (void*)device->sendMessage;
//}

unsigned long xhci_hid_has_keyboard = 0;

unsigned long xhci_get_keyboard(){
	return xhci_hid_has_keyboard;
}

void init_xhci_hid_keyboard(USB_DEVICE* device){
	//
	// Use GET_REPORT protocol
	xhci_hid_has_keyboard = (unsigned long)device;
	
	return;
}

void init_xhci_hid(USB_DEVICE* device){
	unsigned char (*send)(USB_DEVICE* device,TRB setup,TRB data,TRB end) = (void*)device->sendMessage;
	
	printf("[XHCI@HID] Port %x : Is a Human Interface Device\n",device->portnumber);
	if(device->subclass==1){
		printf("[XHCI@HID] Port %x : HID : bootprotocol supported\n",device->portnumber);
	}else if(device->subclass==0){
		printf("[XHCI@HID] Port %x : HID : reportprotocol supported\n",device->portnumber);
	}else{
		printf("[XHCI@HID] Port %x : HID : unknown protocol\n",device->portnumber);
	}
	
	//
	// Use SET_PROTOCOL to boot protocol
	TRB dc1;
	dc1.bar1 = (0x0B<<8) | 0x21;//0x10900; 
	dc1.bar2 = 0;
	dc1.bar3 = 0x8;
	dc1.bar4 = 0x841;
	
	TRB dc2;
	dc2.bar1 = 0;
	dc2.bar2 = 0;
	dc2.bar3 = 0;
	dc2.bar4 = 0;
	
	TRB dc3;
	dc3.bar1 = 0;
	dc3.bar2 = 0;
	dc3.bar3 = 0;
	dc3.bar4 = 1 | (4<<10) | 0x20 | (1 << 16);
	
	unsigned char res = send(device,dc1,dc2,dc3);
	if(res!=1){
		printf("[XHCI@HID] Port %x : HID : SET_PROTOCOL failed with %x \n",device->portnumber,res);
		return;
	}
	
	//
	// Init the rest of the devices
	if(device->protocol==1){
		printf("[XHCI@HID] Port %x : is keyboard\n",device->portnumber);
		init_xhci_hid_keyboard(device);
	}else if(device->protocol==2){
		printf("[XHCI@HID] Port %x : is mouse\n",device->portnumber);
	}else{
		printf("[XHCI@HID] Port %x : unknown HID type %x \n",device->portnumber,device->protocol);
	}
	
}
