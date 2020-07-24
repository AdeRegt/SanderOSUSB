#include "../kernel.h"

#define USB_STORAGE_ENABLE_ENQ 1
#define USB_STORAGE_ENABLE_CAP 0
#define USB_STORAGE_SECTOR_SIZE 512

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
	if((unsigned long)res==EHCI_ERROR){
		return (unsigned char)(EHCI_ERROR&0xFF);
	}
	return res[0];
}


struct cdbres_inquiry {
    unsigned char pdt;
    unsigned char removable;
    unsigned char reserved_02[2];
    unsigned char additional;
    unsigned char reserved_05[3];
    char vendor[8];
    char product[16];
    char rev[4];
} __attribute__ ((packed));

struct cbw_t {
	unsigned long sig;
	unsigned long tag;
	unsigned long xfer_len;
	unsigned char flags;
	unsigned char lun;
	unsigned char wcb_len;
	unsigned char cmd[16];
}  __attribute__ ((packed));

struct rdcap_10_response_t {
	unsigned long max_lba;
	unsigned long blk_size;
} __attribute__ ((packed));

unsigned char* ehci_stick_get_inquiry(unsigned char addr,unsigned char in1){
	unsigned char bufoutsize = 31;
	unsigned char bufinsize = 36;
	struct cbw_t* bufout = (struct cbw_t*)malloc(bufoutsize);
	// 55 53 42 43 e7 3 0 0 24 0 0 0 80 0 c 12 0 0 0 24 0 0 0 0 0 0 0 0 0 0 0
	bufout->lun = 0;
	bufout->tag = 1;
	bufout->sig = 0x43425355;
	bufout->wcb_len = 6; // 0xA -> 12
	bufout->flags = 0x80;
	bufout->xfer_len = 36; // 0x8 -> 36
	bufout->cmd[0] = 0x12;// type is 0x12
	bufout->cmd[1] = 0;
	bufout->cmd[2] = 0;
	bufout->cmd[3] = 0;
	bufout->cmd[4] = bufinsize;
	bufout->cmd[5] = 0;
	unsigned char* bufin = ehci_send_and_recieve_bulk(addr,(unsigned char*)bufout,bufinsize,bufoutsize,in1);
	return bufin;
}

unsigned char* ehci_stick_get_capacity(unsigned char addr,unsigned char in1){
	unsigned char bufoutsize = 31;
	unsigned char bufinsize = 36;
	struct cbw_t* bufout = (struct cbw_t*)malloc(bufoutsize);
	bufout->lun = 0;
	bufout->tag = 1;
	bufout->sig = 0x43425355;
	bufout->wcb_len = 6; // 0xA -> 12
	bufout->flags = 0x80;
	bufout->xfer_len = 36; // 0x8 -> 36
	bufout->cmd[0] = 0x25;// type is 0x12
	bufout->cmd[1] = 0;
	bufout->cmd[2] = 0;
	bufout->cmd[3] = 0;
	bufout->cmd[4] = bufinsize;
	bufout->cmd[5] = 0;
	unsigned char* bufin = ehci_send_and_recieve_bulk(addr,(unsigned char*)bufout,bufinsize,bufoutsize,in1);
	return bufin;
}

unsigned char* ehci_stick_read_sector(unsigned char addr,unsigned char in1, unsigned char lba){
	unsigned long bufoutsize = 31;
	unsigned long bufinsize = USB_STORAGE_SECTOR_SIZE; // 200 120 100 90 80 70 60 50 40 | 36
	struct cbw_t* bufout = (struct cbw_t*)malloc(bufoutsize);
	bufout->lun = 0;
	bufout->tag = 1;
	bufout->sig = 0x43425355;
	bufout->wcb_len = 6; // 0xA -> 12
	bufout->flags = 0x80;
	bufout->xfer_len = 512;
	bufout->cmd[0] = 0x08;// type is 0x12
	bufout->cmd[1] = 0;
	bufout->cmd[2] = 0;
	bufout->cmd[3] = lba;
	bufout->cmd[4] = 1;
	unsigned char* bufin = ehci_send_and_recieve_bulk(addr,(unsigned char*)bufout,bufinsize,bufoutsize,in1);
	return bufin;
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
	if(maxlun==(EHCI_ERROR&0xFF)){
		printf("[SMSD] An error occured while getting max lun \n");	
	}
	printf("[SMSD] Maxlun is %x \n",maxlun);

	// inquiry
	if(USB_STORAGE_ENABLE_ENQ){
		unsigned char* inquiry_raw = ehci_stick_get_inquiry(addr,1);
		struct cdbres_inquiry* inc = (struct cdbres_inquiry*)inquiry_raw;
		if((unsigned long)inquiry_raw==(unsigned long)EHCI_ERROR){
			inquiry_raw = ehci_stick_get_inquiry(addr,0);
			inc = (struct cdbres_inquiry*)inquiry_raw;
			if((unsigned long)inquiry_raw==(unsigned long)EHCI_ERROR){
				printf("[SMSD] An error occured while getting inquiry info \n");
			}
		}
		printf("[SMSD] vendor=%c%c%c%c%c%c%c%c  product=%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c  rev=%c%c%c \n",
			inc->vendor[0],inc->vendor[1],inc->vendor[2],inc->vendor[3],inc->vendor[4],inc->vendor[5],inc->vendor[6],inc->vendor[7],
			inc->product[0],inc->product[1],inc->product[2],inc->product[3],inc->product[4],inc->product[5],inc->product[6],inc->product[7],inc->product[8],inc->product[9],inc->product[10],inc->product[11],inc->product[12],inc->product[13],inc->product[14],inc->product[15],
			inc->rev[0],inc->rev[1],inc->rev[2]
		);
	}

	// get capacity
	if(USB_STORAGE_ENABLE_CAP){
		unsigned char* capacity_raw = ehci_stick_get_capacity(addr,1);
		if((unsigned long)capacity_raw==(unsigned long)EHCI_ERROR){
			capacity_raw = ehci_stick_get_capacity(addr,0);
			if((unsigned long)capacity_raw==(unsigned long)EHCI_ERROR){
				printf("[SMSD] An error occured while getting capacity info \n");
			}
		}
		struct rdcap_10_response_t* capacity = (struct rdcap_10_response_t*)capacity_raw;
		unsigned long maxlba = capacity->max_lba;
		unsigned long blocksize = capacity->blk_size;
		printf("[SMSD] Capacity information: maxlba=%x blocksize=%x \n",maxlba,blocksize);
	}

	unsigned char* t = ehci_stick_read_sector(addr,1,0);
	if((unsigned long)t==(unsigned long)EHCI_ERROR){
		t = ehci_stick_read_sector(addr,0,0);
		if((unsigned long)t==(unsigned long)EHCI_ERROR){
			printf("[SMSD] An error occured while reading a sector \n");
		}
	}
	
	if((unsigned long)t!=(unsigned long)EHCI_ERROR){
		printf("[SMSD] End of reading sector \n");
		for(int i = 0 ; i < USB_STORAGE_SECTOR_SIZE ; i++){printf("%x ",t[i]);}
	}
	for(;;);
}