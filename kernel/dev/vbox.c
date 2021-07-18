#include "../kernel.h"

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE
#define VBOX_VMMDEV_VERSION 0x00010003
#define VBOX_REQUEST_HEADER_VERSION 0x10001
#define VBOX_REQUEST_GET_MOUSE 1
#define VBOX_REQUEST_SET_MOUSE 2
#define VBOX_REQUEST_GUEST_INFO 55
#define VBOX_REPORT_GUEST_INFO 50

#define VBOX_MOUSE_ON (1 << 0) | (1 << 4)
#define VBOX_MOUSE_OFF (0)

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
volatile vbox_mouse_absolute vbox_m;

extern void vbirq();

void irq_vb(){
	outportb(0x20,0x20);
	debugf("VB:DDDD\n");
}

unsigned char is_vbox = 0;

void writer_string_vbox(char *message){
        for(int i = 0 ; i < strlen(message) ; i++){
                outportb(0x504,message[i]);
        }
}

unsigned char is_virtual_box_session_enabled(){
        return is_vbox;
}

void set_virtual_box_session_enabled(unsigned char a){
        is_vbox = a;
}

void vb_mouse_on_off(unsigned int status) {
	vbox_m.header.size = sizeof(vbox_mouse_absolute);
	vbox_m.header.version = VBOX_REQUEST_HEADER_VERSION;
	vbox_m.header.requestType = VBOX_REQUEST_SET_MOUSE;
	vbox_m.header.rc = 0;
	vbox_m.header.reserved1 = 0;
	vbox_m.header.reserved2 = 0;
	vbox_m.features = status;
	vbox_m.x = 0;
	vbox_m.y = 0;
	outportl(vbox_port, (unsigned long)&vbox_m);
}
long vb_mouseinfo[3];

long *vbox_get_mouse_info(){
        vbox_m.header.size = sizeof(vbox_mouse_absolute);
	vbox_m.header.version = VBOX_REQUEST_HEADER_VERSION;
	vbox_m.header.requestType = VBOX_REQUEST_GET_MOUSE;
	vbox_m.header.rc = 0;
	vbox_m.header.reserved1 = 0;
	vbox_m.header.reserved2 = 0;
	outportl(vbox_port, (unsigned long)&vbox_m);

        vb_mouseinfo[0] = vbox_m.x;
        vb_mouseinfo[1] = vbox_m.y;
        vb_mouseinfo[2] = vbox_m.features;
        return (long*)&vb_mouseinfo;
}

void vb_report_existance(){
        vbox_guest_info vbox_e;
        vbox_e.header.size = sizeof(vbox_guest_info);
	vbox_e.header.version = VBOX_REQUEST_HEADER_VERSION;
	vbox_e.header.requestType = VBOX_REPORT_GUEST_INFO;
	vbox_e.header.rc = 0;
	vbox_e.header.reserved1 = 0;
	vbox_e.header.reserved2 = 0;
	vbox_e.version = 0x00010003;
	vbox_e.ostype = 0;
        outportl(vbox_port, (unsigned long)&vbox_e);
}

void init_vbox(unsigned long bus, unsigned long slot, unsigned long function){
        set_virtual_box_session_enabled(1);
	debugf("[VBOX] Virtualbox Guest Additions found!\n");
	for(int i = 0 ; i < 6 ; i++){
		unsigned long addr = getBARaddress(bus,slot,function,0x10+(i*4));
		if(addr){
			debugf("[VBOX] BAR%x : %x \n",i,addr);
		}
	}
	vbox_port = getBARaddress(bus,slot,function,0x10) & 0xFFFFFFF0;
        unsigned long vbox_int = getBARaddress(bus,slot,function,0x3C) & 0x000000FF;
	debugf("[VBOX] Default port is %x with int %x enabled virtualbox logging \n",vbox_port,vbox_int);
        setNormalInt(vbox_int,(unsigned long)vbirq);

        vb_report_existance();
        debugf("[VBOX] Told host about us\n");

        vb_mouse_on_off(VBOX_MOUSE_ON);
        debugf("[VBOX] Enable mouse\n");

}
