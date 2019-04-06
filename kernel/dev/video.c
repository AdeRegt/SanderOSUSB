#include "../kernel.h"
unsigned char* videomemory = (unsigned char*)0xb8000;
//
// STRING
//
//


int vidpnt = 0;
int curx = 0;
int cury = 0;

void init_video(){
	// set cursor shape
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, (inportb(0x3D5) & 0xC0) | 0);
	outportb(0x3D4, 0x0B);
	outportb(0x3D5, (inportb(0x3D5) & 0xE0) | 15);
	// set cursor location
	vidpnt = 0;
	curx = 0;
	cury = 0;
}

void printstring(char* message){
	int a = 0;
	char b = 0;
	while((b=message[a++])!=0x00){
		putc(b);
	}
}

void putc(char a){
	if(a!='\n'){
		vidpnt = (curx*2)+(160*cury);
		videomemory[vidpnt++] = a;
		videomemory[vidpnt++] = 0x07;
		curx++;
	}
	if(curx==80||a=='\n'){
		curx = 0;
		cury++;
	}
	if(cury==25){
		cury = 24;	
		curx = 0;
		for(int i = 0 ; i < 24 ; i++){
			int v1dpnt = (160*(0+i));
			int v2dpnt = (160*(1+i));
			for(int z = 0 ; z < 160 ; z++){
				videomemory[v1dpnt+z] = videomemory[v2dpnt+z];
				videomemory[v2dpnt+z] = 0x00;
			}
		}
	}
}


void printf(char* format,...) 
{ 
	char *traverse; 
	unsigned int i; 
	signed int t;
	char *s; 
	
	//Module 1: Initializing Myprintf's arguments 
	va_list arg; 
	va_start(arg, format); 
	
	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		while( *traverse != '%' && *traverse != '\0' ) 
		{ 
			putc(*traverse);
			traverse++; 
		} 
		if(*traverse =='\0'){
		    break; 
		}
		traverse++; 
		
		//Module 2: Fetching and executing arguments
		switch(*traverse) 
		{ 
			case 'c' : i = va_arg(arg,int);		//Fetch char argument
						putc(i);
						break; 
						
			case 'd' : t = va_arg(arg,int); 		//Fetch Decimal/Integer argument
						if(t<0) 
						{ 
							t = -t;
							putc('-'); 
						} 
						printstring(convert(t,10));
						break; 
						
			case 'o': i = va_arg(arg,unsigned int); //Fetch Octal representation
						printstring(convert(i,8));
						break; 
						
			case 's': s = va_arg(arg,char *); 		//Fetch string
						printstring(s); 
						break; 
						
			case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
						printstring(convert(i,16));
						break; 
				
		}	
	} 
	
	//Module 3: Closing argument list to necessary clean-up
	va_end(arg); 
} 
 
char *convert(unsigned int num, int base) 
{ 
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50]; 
	char *ptr; 
	
	ptr = &buffer[49]; 
	*ptr = '\0'; 
	
	do 
	{ 
		*--ptr = Representation[num%base]; 
		num /= base; 
	}while(num != 0); 
	
	return(ptr); 
}

// FROM: https://wiki.osdev.org/Printing_To_Screen
char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void hexdump(unsigned long a){
	char msg[10];
	itoa(a,msg,16);
	printstring(msg);
}

void putpixel(int x,int y, int color) {
	unsigned char* screen = (unsigned char*)0xA0000;
    	unsigned where = x + (y*200);
    	screen[where] = color;
}

void cls(){
	for(int x = 0 ; x < 200 ; x++){
		for(int y = 0 ; y < 320 ; y++){
			putpixel(x,y,0x01);
		}
	}
}

//
// SOURCE: https://forum.osdev.org/viewtopic.php?f=1&t=10534
//

typedef char byte;
typedef unsigned short word;
typedef unsigned long dword;

#define SZ(x) (sizeof(x)/sizeof(x[0]))


// misc out (3c2h) value for various modes

#define R_COM  0x63 // "common" bits

#define R_W256 0x00
#define R_W320 0x00
#define R_W360 0x04
#define R_W376 0x04
#define R_W400 0x04

#define R_H200 0x00
#define R_H224 0x80
#define R_H240 0x80
#define R_H256 0x80
#define R_H270 0x80
#define R_H300 0x80
#define R_H360 0x00
#define R_H400 0x00
#define R_H480 0x80
#define R_H564 0x80
#define R_H600 0x80


static const byte hor_regs [] = { 0x0,  0x1,  0x2,  0x3,  0x4, 0x5,  0x13 };

static const byte width_256[] = { 0x5f, 0x3f, 0x40, 0x82, 0x4a,0x9a, 0x20 };
static const byte width_320[] = { 0x5f, 0x4f, 0x50, 0x82, 0x54,0x80, 0x28 };
static const byte width_360[] = { 0x6b, 0x59, 0x5a, 0x8e, 0x5e,0x8a, 0x2d };
static const byte width_376[] = { 0x6e, 0x5d, 0x5e, 0x91, 0x62,0x8f, 0x2f };
static const byte width_400[] = { 0x70, 0x63, 0x64, 0x92, 0x65,0x82, 0x32 };

static const byte ver_regs  [] = { 0x6,  0x7,  0x9,  0x10, 0x11,0x12, 0x15, 0x16 };

static const byte height_200[] = { 0xbf, 0x1f, 0x41, 0x9c, 0x8e,0x8f, 0x96, 0xb9 };
static const byte height_224[] = { 0x0b, 0x3e, 0x41, 0xda, 0x9c,0xbf, 0xc7, 0x04 };
static const byte height_240[] = { 0x0d, 0x3e, 0x41, 0xea, 0xac,0xdf, 0xe7, 0x06 };
static const byte height_256[] = { 0x23, 0xb2, 0x61, 0x0a, 0xac,0xff, 0x07, 0x1a };
static const byte height_270[] = { 0x30, 0xf0, 0x61, 0x20, 0xa9,0x1b, 0x1f, 0x2f };
static const byte height_300[] = { 0x70, 0xf0, 0x61, 0x5b, 0x8c,0x57, 0x58, 0x70 };
static const byte height_360[] = { 0xbf, 0x1f, 0x40, 0x88, 0x85,0x67, 0x6d, 0xba };
static const byte height_400[] = { 0xbf, 0x1f, 0x40, 0x9c, 0x8e,0x8f, 0x96, 0xb9 };
static const byte height_480[] = { 0x0d, 0x3e, 0x40, 0xea, 0xac,0xdf, 0xe7, 0x06 };
static const byte height_564[] = { 0x62, 0xf0, 0x60, 0x37, 0x89,0x33, 0x3c, 0x5c };
static const byte height_600[] = { 0x70, 0xf0, 0x60, 0x5b, 0x8c,0x57, 0x58, 0x70 };

// the chain4 parameter should be 1 for normal 13h-type mode, but 
// only allows 320x200 256x200, 256x240 and 256x256 because you
// can only access the first 64kb

// if chain4 is 0, then plane mode is used (tweaked modes), and
// you'll need to switch planes to access the whole screen but
// that allows you using any resolution, up to 400x600

int init_graph_vga(int width, int height,int chain4) 
  // returns 1=ok, 0=fail
{
   const byte *w,*h;
   byte val;
   int a;

   switch(width) {
      case 256: w=width_256; val=R_COM+R_W256; break;
      case 320: w=width_320; val=R_COM+R_W320; break;
      case 360: w=width_360; val=R_COM+R_W360; break;
      case 376: w=width_376; val=R_COM+R_W376; break;
      case 400: w=width_400; val=R_COM+R_W400; break;
      default: return 0; // fail
   }
   switch(height) {
      case 200: h=height_200; val|=R_H200; break;
      case 224: h=height_224; val|=R_H224; break;
      case 240: h=height_240; val|=R_H240; break;
      case 256: h=height_256; val|=R_H256; break;
      case 270: h=height_270; val|=R_H270; break;
      case 300: h=height_300; val|=R_H300; break;
      case 360: h=height_360; val|=R_H360; break;
      case 400: h=height_400; val|=R_H400; break;
      case 480: h=height_480; val|=R_H480; break;
      case 564: h=height_564; val|=R_H564; break;
      case 600: h=height_600; val|=R_H600; break;
      default: return 0; // fail
   }

   // chain4 not available if mode takes over 64k

   if(chain4 && (long)width*(long)height>65536L) return 0; 

   // here goes the actual modeswitch

   outportb(0x3c2,val);
   outportw(0x3d4,0x0e11); // enable regs 0-7

   for(a=0;a<SZ(hor_regs);++a) 
      outportw(0x3d4,(word)((w[a]<<8)+hor_regs[a]));
   for(a=0;a<SZ(ver_regs);++a)
      outportw(0x3d4,(word)((h[a]<<8)+ver_regs[a]));

   outportw(0x3d4,0x0008); // vert.panning = 0

   if(chain4) {
      outportw(0x3d4,0x4014);
      outportw(0x3d4,0xa317);
      outportw(0x3c4,0x0e04);
   } else {
      outportw(0x3d4,0x0014);
      outportw(0x3d4,0xe317);
      outportw(0x3c4,0x0604);
   }

   outportw(0x3c4,0x0101);
   outportw(0x3c4,0x0f02); // enable writing to all planes
   outportw(0x3ce,0x4005); // 256color mode
   outportw(0x3ce,0x0506); // graph mode & A000-AFFF

   inportb(0x3da);
   outportb(0x3c0,0x30); outportb(0x3c0,0x41);
   outportb(0x3c0,0x33); outportb(0x3c0,0x00);

   for(a=0;a<16;a++) {    // ega pal
      outportb(0x3c0,(byte)a); 
      outportb(0x3c0,(byte)a); 
   } 
   
   outportb(0x3c0, 0x20); // enable video

   return 1;

}

