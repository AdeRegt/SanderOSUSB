#include "stdarg.h"
#define ATAPI_SECTOR_SIZE 2048



typedef struct
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
	unsigned long mmap_length;
	unsigned long mmap_addr;
	unsigned long drives_length;
	unsigned long drives_addr;
	unsigned long config_table;
	unsigned long boot_loader_name;
	unsigned long apm_table;
	unsigned long vbe_control_info;
	unsigned long vbe_mode_info;
	unsigned long vbe_mode;
	unsigned long vbe_interface_seg;
	unsigned long vbe_interface_off;
	unsigned long vbe_interface_len;
}GRUBMultiboot ;

typedef struct{
	unsigned char keyboard;
	unsigned char usb;
}GRUBStatus;

GRUBStatus getGrubStatus();

/**
 * printf own implementation
 **/
void printf(char *,...);

/**
 * Convert integer decimal base number into octal, hex, etc.
 * @param number
 *  number to convert
 * @param int
 *  base to convert to
 **/
char* convert(unsigned int num, int base);

/**
 * reads a byte from the input port specified
 **/
unsigned char inportb (unsigned short _port);

/**
 * write a byte data to the specified output port
 **/
void outportb (unsigned short _port, unsigned char _data);

/**
 * reads 2 bytes from the input port specified
 **/
unsigned short inportw(unsigned short _port);

/**
 * write 2 bytes data to the specified output port
 **/
void outportw(unsigned short _port, unsigned short _data);

/**
 * reads 8 bytes data to the specified output port
 **/
unsigned long inportl(unsigned short _port);

/**
 * write 8 bytes data to the specified output port
 **/
void outportl(unsigned short _port, unsigned long _data);

/**
 * Entry point of the kernel
 **/
void kernel_main(GRUBMultiboot *grub, unsigned long magic);

/**
 * empty loop that sleeps for 'ms' miliseconds
 **/
void sleep(int ms);

/**
 * Initialize the ACP interface (mostly) to control the power
 * https://en.wikipedia.org/wiki/Advanced_Configuration_and_Power_Interface
 **/
void init_acpi();

/**
 * self explained...
 * NOTE: Should be called only after init_acpi which occurs at the begining
 * but just in case
 **/
void poweroff();

// MEMORY NOTE: Do they comply with the standard?
void *malloc(unsigned long size);
void *memset(void *str, int c, int n);
int memcmp( char *str1, char *str2, int n);
void *malloc_align(unsigned long size,unsigned long tag);
void free(void *loc);
int strlen(char *str);
int memcmp( char *str1, char *str2, int size);
void memcpy( char *str1,  char *str2, int size);

void cpuid_get_details();

// BLOCKDEVICE
//void init_blockdevice();
//void introduceDevice(unsigned char type,unsigned long pointertodevice,unsigned long readsector,unsigned long writesector,unsigned long eject,unsigned long framesize,unsigned char* name);
//void dirdev();

/**
 * Renders a string [same as printf(msg)]
 **/
void printstring(char* msg);
/**
 * Renders a char
 **/
void putc(char a);
/**
 * Initialize the video driver
 **/
void init_video();
/**
 * Prints an unsigned integer
 **/
void hexdump(unsigned long msg);
/**
 * The actual render of the character into the screen buffer
 **/
void drawcharraw(unsigned char c, int offsetX, int offsetY, int fgcolor, int bgcolor);
/**
 * Gets the character from keyboard input
 **/
unsigned char getch();
void curset(int x,int y);
void setForeGroundBackGround(unsigned char fg,unsigned char bg);

char* vsprintf(char* format,va_list arg);
char* sprintf(char* format,...);
void debugf(char* format,...);
void writer_string_serial(char* message,unsigned short PORT);
unsigned short getDefaultSerialPort();

//////
// arrows key codes
//////
#define VK_UP 0xCB
#define VK_LEFT 0xCC
#define VK_RIGHT 0xCD
#define VK_DOWN 0xCE

typedef struct{
	int mouse_x;
	int mouse_y;
	int mouse_z;
	int mousePressed;
	int keyPressed;
}InputStatus;
InputStatus getInputStatus();

typedef struct
{
	unsigned short command;
	unsigned short control;
	unsigned char irq;
	unsigned char slave;
} IDEDevice;

// VIDEO
void force_mouse_resample();
void print_bios_char_table();
int init_graph_vga(unsigned int width, unsigned int height,int chain4);
int isGraphicsMode();
void putpixel(int x,int y, unsigned char color);
char getpixel(int x,int y);
void cls();
void draw();
void draw_bmp(unsigned char* file_buffer, unsigned short offsetX, unsigned short offsetY);
void addController(unsigned char drawable,unsigned long drawablefunc,unsigned short x,unsigned short y,unsigned short w,unsigned short h,unsigned long value,unsigned long onSelected,unsigned long onFocus,unsigned char controller);
unsigned char* prompt(char *message,int maxinput);
void message(char *message);
char confirm(char *message);
char choose(char *message,int argcount,char **args);
char *browseDIR(char *path);
char *browse();
unsigned long show();
void freeGui();

void init_vbox(unsigned long bus, unsigned long slot, unsigned long function);
unsigned char is_virtual_box_session_enabled();
void writer_string_vbox(char *message);
long *vbox_get_mouse_info();
void update_drawable_mouse();

/** 
 * Initialize GDT
 * NOTE: is it Global descritpor table?
 * https://en.wikipedia.org/wiki/Global_Descriptor_Table
 **/
void init_gdt();

/** 
 * Initialize IDT
 * NOTE: is it Interrup descritpor table?
 * https://en.wikipedia.org/wiki/Interrupt_descriptor_table
 **/
void init_idt();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void setErrorInt(unsigned char num,unsigned long base);
void setNormalInt(unsigned char num,unsigned long base);


////////
// PROCESSING
///////

// PAGING
void init_paging();
void set_paging_frame(unsigned long addr);

// MULTITASKING
typedef struct{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
}Register;
typedef struct {
    Register registers;
    unsigned char active;
    int timer;
	int ticks;
}Process;
Process *getCurrentProcess();
void init_multitasking();
int createTask(unsigned long entrypoint);
void killTask(int pid);
int activeTask();

// TIMER MOD
void init_timer();
int getTicks();
void resetTicks();

//////////////
// DEVICES
//////////////

// SOUNDBLASTER
void init_soundblaster16();

// PS2
void init_ps2();

// PCI
void init_pci();
unsigned long getBARaddress(int bus,int slot,int function,int barNO);
void setBARaddress(int bus,int slot,int function,int barNO,unsigned long result);
unsigned long getPCIConfiguration(int bus,int slot,int function);
void dumpPCIConfiguration(int bus,int slot,int function);
char pci_enable_busmastering_when_needed(int bus,int slot,int function);
void init_xhci(unsigned long bus,unsigned long slot,unsigned long function);
void dumpPCI(int bus,int slot,int function);
void memdump(unsigned long location);

// SERIAL
void init_serial();

// IDE
void init_ide(unsigned short BAR);
void init_ide2();
void ahci_init(int bus,int slot,int function);
void uhci_init(int bus,int slot,int function);


typedef struct{
	//
	// Blockdevice settings
	//
	unsigned long readRawSector;
	unsigned long writeRawSector;
	unsigned long reinitialise;
	unsigned long eject;
	
	//
	// Filesystem settings
	//
	unsigned long dir;
	unsigned long readFile;
	unsigned long writeFile;
	unsigned long existsFile;
	unsigned long newFile;
	unsigned long deleteFile;
	unsigned long renameFile;
	unsigned long copyfile;
	
	//
	// Misc
	//
	unsigned char readonly;
	
	//
	// Advanced
	//
	unsigned long arg1;	// LINK TO DATAPOINTER
	unsigned long arg2;	// OFFSET DISK
	unsigned long arg3;
	unsigned long arg4;
	unsigned long arg5;	// SIZEOF SECTOR
	unsigned long arg6;
	unsigned long arg7;
	unsigned long arg8;
	unsigned long arg9;
}Device;

Device *getNextFreeDevice();
char* dir(char* path);
char fexists(unsigned char* path);
int fread(char* path,unsigned char* buffer);
int fwrite(char* path,unsigned char* buffer,unsigned long filesize);
void raw_write(Device *device, unsigned long LBA, unsigned char count, unsigned short *location);
int getDeviceCount();
int iself(unsigned char* buffer);
unsigned long loadelf(void * buffer);
void detectFilesystemsOnMBR(Device* dev);
void initialiseExt2(Device* device);
void initialiseFAT(Device* device);
void initialiseSFS(Device *device);

int pow(int base,int exp);

void irq_timer();

typedef struct{
	unsigned long bar1;
	unsigned long bar2;
	unsigned long bar3;
	unsigned long bar4;
}TRB;

typedef struct{
	unsigned char drivertype; // 1,2,3
	unsigned char portnumber;
	unsigned long localring;
	unsigned long localringoffset;
	
	int assignedSloth;
	
	unsigned char class;
	unsigned char subclass;
	unsigned char protocol;
	
	unsigned long sendMessage;
	unsigned long sendBulk;
	unsigned long recieveBulk;
	unsigned char endpointControl;
	unsigned char endpointBulkIN;
	unsigned char endpointBulkOUT;
}USB_DEVICE;
void init_ehci(unsigned long bus,unsigned long slot,unsigned long function);
void init_usb_hid(USB_DEVICE* device);
unsigned long usb_get_keyboard();
unsigned char get_usb_hid_keyboard_input(USB_DEVICE* device,unsigned char wait);



/////////////
// ETHERNET
/////////////

typedef struct{
	unsigned long buffersize;
	unsigned long low_buf;
	unsigned long high_buf;
}PackageRecievedDescriptor;

typedef struct{
	unsigned long sendPackage;
	unsigned long recievePackage;
	unsigned char is_enabled;
	unsigned char mac[8];
	volatile unsigned long is_online;
}EthernetDevice;

void init_rtl(int bus,int slot,int function);
void ethernet_detect(int bus,int slot,int function,int device,int vendor);
void ethernet_set_link_status(unsigned long a);
void register_ethernet_device(unsigned long sendPackage,unsigned long recievePackage,unsigned char mac[8]);
EthernetDevice getDefaultEthernetDevice();
PackageRecievedDescriptor getEthernetPackage();
int sendEthernetPackage(PackageRecievedDescriptor desc,unsigned char first,unsigned char last,unsigned char ip,unsigned char udp, unsigned char tcp);
void initialise_ethernet();
//void dirdev();

typedef struct  {
    unsigned char bRequestType;
    unsigned char bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
} EhciCMD;

typedef struct {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
	unsigned short wMaxPacketSize;
	unsigned char bInterval;
}EHCI_DEVICE_ENDPOINT;


#define EHCI_PERIODIC_FRAME_SIZE    1024

typedef struct {
    volatile unsigned long nextlink;
    volatile unsigned long altlink;
    volatile unsigned long token;
    volatile unsigned long buffer[5];
    volatile unsigned long extbuffer[5];
}EhciTD;

typedef struct {
    volatile unsigned long horizontal_link_pointer;
    volatile unsigned long characteristics;
    volatile unsigned long capabilities;
    volatile unsigned long curlink;

    volatile unsigned long nextlink;
    volatile unsigned long altlink;
    volatile unsigned long token;
    volatile unsigned long buffer[5];
    volatile unsigned long extbuffer[5];
    
}EhciQH;

typedef struct __attribute__ ((packed)){
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned short wTotalLength;
    unsigned char  bNumInterfaces;
    unsigned char  bConfigurationValue;
    unsigned char  iConfiguration;
    unsigned char  bmAttributes;
    unsigned char  bMaxPower;
}usb_config_descriptor ;

typedef struct __attribute__ ((packed)) {
    unsigned char  bLength;
    unsigned char  bDescriptorType;

    unsigned char  bInterfaceNumber;
    unsigned char  bAlternateSetting;
    unsigned char  bNumEndpoints;
    unsigned char  bInterfaceClass;
    unsigned char  bInterfaceSubClass;
    unsigned char  bInterfaceProtocol;
    unsigned char  iInterface;
}usb_interface_descriptor;

void usb_stick_init(USB_DEVICE *device);//unsigned char addr,unsigned char subclass,unsigned char protocol);
#define EHCI_ERROR 0xCAFEBABE
unsigned char* ehci_send_and_recieve_command(unsigned char addr,EhciCMD* commando, void *buffer);
unsigned char* ehci_send_and_recieve_bulk(USB_DEVICE *device,unsigned char* out,unsigned long expectedIN,unsigned long expectedOut);
unsigned char* ehci_recieve_bulk(USB_DEVICE *device,unsigned long expectedIN,void *buffer);
unsigned long ehci_send_bulk(USB_DEVICE *device,unsigned char* out,unsigned long expectedOut);
unsigned char* ehci_get_device_configuration(unsigned char addr,unsigned char size);

unsigned long usb_send_bulk(USB_DEVICE *device,unsigned long count,void *buffer);
unsigned long usb_recieve_bulk(USB_DEVICE *device,unsigned long count,void *commando);
void *usb_send_and_recieve_control(USB_DEVICE *device,void *commando,void *buffer);
void usb_device_install(USB_DEVICE *device);

typedef struct {
	void *next;
	void *previous;
	void *locationToItem;
}ArrayListElement;

#define PATH_MAX 100
char *getcwd();
void setcwd(char *t);

void init_e1000(int bus,int slot,int function);
unsigned short pciConfigReadWord (unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
void pciConfigWriteWord (unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset,unsigned long value);

void init_cmos();
unsigned char* cmos_update_datetime_to_variables();
