#include "../kernel.h"

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE
#define VBOX_VMMDEV_VERSION 0x00010003
#define VBOX_REQUEST_HEADER_VERSION 0x10001
#define VBOX_REQUEST_GET_MOUSE 1
#define VBOX_REQUEST_SET_MOUSE 2
#define VBOX_REQUEST_GUEST_INFO 55

typedef struct {
        unsigned long size;
        unsigned long version;
        unsigned long requestType;
        long  rc;
        unsigned long reserved1;
        unsigned long reserved2;
} vbox_header;

typedef struct {
        vbox_header header;
        unsigned long version;
        unsigned long ostype;
} vbox_guest_info;

typedef struct  {
        vbox_header header;
        unsigned long features;
        long x;
        long y;
} vbox_mouse_absolute; 
 

static int vbox_port;

extern void vbirq();

void irq_vb(){
	outportb(0x20,0x20);
	debugf("VB:DDDD\n");
}

void init_vbox(unsigned long bus, unsigned long slot, unsigned long function){
	debugf("[VBOX] Virtualbox Guest Additions found!\n");
	for(int i = 0 ; i < 6 ; i++){
		unsigned long addr = getBARaddress(bus,slot,function,0x10+(i*4));
		if(addr){
			debugf("[VBOX] BAR%x : %x \n",i,addr);
		}
	}
	vbox_port = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFF0;
	debugf("[VBOX] Default port is %x with default value %x \n",vbox_port,inportl(vbox_port));
}
