#include "../kernel.h"

//
// In this file all calls to one of the USB drivers are listed
void *usb_send_bulk(USB_DEVICE *device,unsigned long count,void *buffer){
	if(device->drivertype==1){
		return (void *)EHCI_ERROR;
	}else if(device->drivertype==2){
		return (void *) ehci_send_bulk(device,buffer,count);
	}else if(device->drivertype==3){
		return (void *)EHCI_ERROR;
	}else{
		return (void *)EHCI_ERROR;
	}
}

void *usb_recieve_bulk(USB_DEVICE *device,unsigned long count,void *commando){
	if(device->drivertype==1){
		return (void*)EHCI_ERROR;
	}else if(device->drivertype==2){
		return (void *) ehci_recieve_bulk(device,count,commando);
	}else if(device->drivertype==3){
		return (void*)EHCI_ERROR;
	}else{
		return (void*)EHCI_ERROR;
	}
}

void *usb_send_and_recieve_control(USB_DEVICE *device,void *commando,void *buffer){
	if(device->drivertype==1){
		return (void *)EHCI_ERROR;
	}else if(device->drivertype==2){
		return (void *) ehci_send_and_recieve_command(device->portnumber,commando,buffer);
	}else if(device->drivertype==3){
		return (void *)EHCI_ERROR;
	}else{
		return (void *)EHCI_ERROR;
	}
}

void usb_device_install(USB_DEVICE *device){
	if(device->class==8){
        printf("[USB] Port %x : Mass Storage Device detected!\n",device->portnumber);
        usb_stick_init(device);
    }else if(device->class==3){
        printf("[USB] Port %x : Human Interface Device detected!\n",device->portnumber);
		init_usb_hid(device);
    }else{
        printf("[USB] Port %x : Unable to understand deviceclass: %x \n",device->portnumber,device->class);
    }
}