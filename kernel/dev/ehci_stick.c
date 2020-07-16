#include "../kernel.h"

unsigned char ehci_stick_get_max_lun(unsigned char addr){
	EhciCMD* commando = (EhciCMD*) malloc_align(sizeof(EhciCMD),0x1FF);
	commando->bRequest = 0xFE; // get_max_lun
    commando->bRequestType |= 0x80; // dir=IN
	commando->bRequestType |= (0x01 << 5); // TYPE= class
	commando->bRequestType |= 1; // TO= interface
    commando->wIndex = 0; // windex=0
    commando->wLength = 1; // getlength=8
    commando->wValue = 0; // get config info
	unsigned char *res = ehci_send_and_recieve_command(addr,commando);
	if(res==EHCI_ERROR){
		return EHCI_ERROR;
	}
	return res[0];
}

//
// On emulator, the value is:
// subclass= 0x06
// protocol= 0x50
void ehci_stick_init(unsigned char addr,unsigned char subclass,unsigned char protocol){
	printf("[SMSD] Reached USB Mass Storage Device endpoint. subclass=%x protocol=%x \n",subclass,protocol);
	if(!(subclass==0x02||subclass==0x05||subclass==0x06)){
		printf("[SMSD] Unsupported subclass version. Requested 2/5/6, found 0x%x \n",subclass);
		return;
	}
	if(protocol!=0x50){
		printf("[SMSD] Unsupported protocol version. Requested 0x50, found 0x%x \n",protocol);
		return;
	}

	// get maxlun
	unsigned char maxlun = ehci_stick_get_max_lun(addr);
	if(EHCI_ERROR==maxlun){
		printf("[SMSD] An error occured while getting max lun \n");	
	}
	printf("[SMSD] Maxlun is %x \n",maxlun);

	// get capacity
	for(;;);
}