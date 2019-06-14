#include "../kernel.h"

#define VBOX_VENDOR_ID 0x80EE
#define VBOX_DEVICE_ID 0xCAFE
#define VBOX_VMMDEV_VERSION 0x00010003
#define VBOX_REQUEST_HEADER_VERSION 0x10001
#define VBOX_REQUEST_GET_MOUSE 1
#define VBOX_REQUEST_SET_MOUSE 2
#define VBOX_REQUEST_GUEST_INFO 50

struct vbox_header {
        unsigned long size;
        unsigned long version;
        unsigned long requestType;
        long  rc;
        unsigned long reserved1;
        unsigned long reserved2;
};

struct vbox_guest_info {
        struct vbox_header header;
        unsigned long version;
        unsigned long ostype;
};

struct vbox_mouse_absolute {
        struct vbox_header header;
        unsigned long features;
        long x;
        long y;
};
 
static struct vbox_mouse_absolute vbox_mouse_phys;
static struct vbox_mouse_absolute * vbox_mouse;

static int vbox_port;
//static unsigned long * vbox_vmmdev;
//static struct vbox_header guest_info;
struct vbox_guest_info hdr;

extern void vbirq();

void irq_vb(){
	outportb(0x20,0x20);
	printf("VB:DDDD\n");
}

void init_vbox(unsigned long BAR ,unsigned char irq){
	printf("VBOX: port detected at %x with irq %x \n",BAR,irq);
	
	setNormalInt(irq,(unsigned long)vbirq);
	
	struct vbox_guest_info * guest_info = (struct vbox_guest_info *) &hdr;
	guest_info->header.size = sizeof(struct vbox_guest_info);
	guest_info->header.version = VBOX_REQUEST_HEADER_VERSION;
	guest_info->header.requestType = VBOX_REQUEST_GUEST_INFO;
	guest_info->header.rc = 0;
	guest_info->header.reserved1 = 0;
	guest_info->header.reserved2 = 0;
	guest_info->version = VBOX_VMMDEV_VERSION;
	guest_info->ostype = 0;
	
	outportl(vbox_port, (unsigned long)&hdr);
	
	vbox_mouse = (struct vbox_mouse_absolute *)&vbox_mouse_phys;
	vbox_mouse->header.size = sizeof(struct vbox_mouse_absolute);
	vbox_mouse->header.version = VBOX_REQUEST_HEADER_VERSION;
	vbox_mouse->header.requestType = VBOX_REQUEST_SET_MOUSE;
	vbox_mouse->header.rc = 0;
	vbox_mouse->header.reserved1 = 0;
	vbox_mouse->header.reserved2 = 0;
	vbox_mouse->features = (1 << 0) | (1 << 4);
	vbox_mouse->x = 0;
	vbox_mouse->y = 0;
	outportl(vbox_port, (unsigned long)&vbox_mouse_phys);

	vbox_mouse->header.requestType = VBOX_REQUEST_GET_MOUSE;
	
}
