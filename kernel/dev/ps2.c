#include "../kernel.h"

//
// PS2
//
//

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64
#define PS2_TIMEOUT 10

#define PS2_MOUSE_MIN_X 1
#define PS2_MOUSE_MIN_Y 1
#define PS2_MOUSE_MIN_W 312
#define PS2_MOUSE_MIN_H 190

#define PS2_MOUSE_CUR_W 8
#define PS2_MOUSE_CUR_H 6


char getPS2StatusRegisterText(){
	return inportb(PS2_STATUS);
}

int getPS2ReadyToRead(){
	return getPS2StatusRegisterText() & 0b00000001;
}

int getPS2ReadyToWrite(){
	return getPS2StatusRegisterText() & 0b00000010;
}

int writeToFirstPS2Port(unsigned char data){
	resetTicks();
	while(getPS2ReadyToWrite()>0){
		if(getTicks()==PS2_TIMEOUT){
			return 0;
		}
	}
	outportb(PS2_DATA,data);
	return 1;
}

int writeToSecondPS2Port(unsigned char data){
	outportb(0x64,0xD4);
	resetTicks();
	while(getPS2ReadyToWrite()>0){
		if(getTicks()==PS2_TIMEOUT){return 0;}
	}
	outportb(0x60,data);
	return 1;
}

int waitforps2ok(){
	resetTicks();
	while(inportb(PS2_DATA)!=0xFA){
		if(getTicks()==PS2_TIMEOUT){
			return 0;
		}
	}
	return 1;
}

int ps2_echo(){
	int a = writeToFirstPS2Port(0xEE);
	if(a==0){
		return 0;
	}
	resetTicks();
	while(inportb(PS2_DATA)!=0xEE){
		if(getTicks()==PS2_TIMEOUT){
			return 0;
		}
	}
	return 1;
}

void printps2devicetype(unsigned char a){
	if(a==0x00){
		printf("PS2: standard ps/2 mouse\n");
	}else if(a==0x03){
		printf("PS2: mouse with scroll wheel\n");
	}else if(a==0x04){
		printf("PS2: 5 button mouse\n");
	}else if(a==0xab||a==0x41||a==0xab||a==0xc1||a==0x83){
		printf("PS2: keyboard\n");
	}else{
		printf("PS2: unknown: %x ",a); 
	}
}

extern void mouseirq();
extern void keyboardirq();

int csr_y = 12;
int csr_x = 40;
volatile int csr_t = 0;
volatile int ccr_x = 50;
volatile int ccr_y = 50;
volatile int ccr_a = 0;
volatile int ccr_b = 0;
volatile int oldx = 0;
volatile int oldy = 0;
volatile char old00z =0;
volatile char old01z =0;
volatile char old02z =0;
volatile char old10z =0;
volatile char old11z =0;
volatile char old12z =0;
volatile char old20z =0;
volatile char old21z =0;
volatile char old22z =0;
volatile int clck = 0;

volatile int mouse_cycle = 0;
volatile char mouse_byte[5];
volatile char mousetype = 0;
unsigned char mousebuffer[6*8];

volatile char forcemouseresample = 1;

void force_mouse_resample(){
	forcemouseresample = 1;
}

void irq_mouse(){
	unsigned char status = inportb(0x64);
	if(status & 1){
		char sts = inportb(0x60);
		if(status & 0x20){
			switch(mouse_cycle){
				case 0:
					mouse_byte[0] = sts;
					++mouse_cycle;
					break;
				case 1:
					mouse_byte[1] = sts;
					++mouse_cycle;
					break;
				case 2:
					mouse_byte[2] = sts;
					mouse_cycle = 0;
					if(mousetype==0){
						if(mouse_byte[0]==0x08){
							mousetype=1;
						}else if(mouse_byte[1]==0x08){
							mousetype=2;
						}
					}
					if(mousetype){
						if(mousetype==1){
							ccr_x += mouse_byte[1];
							ccr_y += mouse_byte[2];
							if(mouse_byte[0]&0x01){
								clck = 1;
							}
							if(mouse_byte[0]&0x02){
								clck = 2;
							}
							if(mouse_byte[0]&0x04){
								clck = 3;
							}
						}else{
							ccr_x += mouse_byte[0];
							ccr_y += mouse_byte[2];
							if(mouse_byte[1]&0x01){
								clck = 1;
							}
							if(mouse_byte[1]&0x02){
								clck = 2;
							}
							if(mouse_byte[1]&0x04){
								clck = 3;
							}
						}
					}
					break;
			}
		}
	}
	
	if(ccr_x<PS2_MOUSE_MIN_X){
		ccr_x = PS2_MOUSE_MIN_X;
	}
	if(ccr_y<PS2_MOUSE_MIN_Y){
		ccr_y = PS2_MOUSE_MIN_Y;
	}
	if(ccr_x>PS2_MOUSE_MIN_W){
		ccr_x = PS2_MOUSE_MIN_W;
	}
	if(ccr_y>PS2_MOUSE_MIN_H){
		ccr_y = PS2_MOUSE_MIN_H;
	}

	unsigned char mouse[PS2_MOUSE_CUR_H] = {
	0b11111000,
	0b11110000,
	0b11111000,
	0b11111100,
	0b10011110,
	0b00001100
	};

	// set old buffer back
	if(mousebuffer[0]!=0x00&&forcemouseresample==0){
		for(int y = 0 ; y < PS2_MOUSE_CUR_H ; y++){
			for(int x = 0 ; x < PS2_MOUSE_CUR_W ; x++){
				putpixel(oldx+x,oldy+y,mousebuffer[(y*PS2_MOUSE_CUR_W)+x]);
			}
		}
	}
	forcemouseresample = 0;

	// put new buffer
	for(int y = 0 ; y < PS2_MOUSE_CUR_H ; y++){
		for(int x = 0 ; x < PS2_MOUSE_CUR_W ; x++){
			mousebuffer[(y*PS2_MOUSE_CUR_W)+x] = getpixel(ccr_x+x,ccr_y+y);
		}
	}

	for(int y = 0 ; y < PS2_MOUSE_CUR_H ; y++){
		unsigned char mouseline = mouse[y];
		if(mouseline & 0b10000000){
			putpixel(ccr_x+0,ccr_y+y,10);
		}
		if(mouseline & 0b01000000){
			putpixel(ccr_x+1,ccr_y+y,10);
		}
		if(mouseline & 0b00100000){
			putpixel(ccr_x+2,ccr_y+y,10);
		}
		if(mouseline & 0b00010000){
			putpixel(ccr_x+3,ccr_y+y,10);
		}
		if(mouseline & 0b00001000){
			putpixel(ccr_x+4,ccr_y+y,10);
		}
		if(mouseline & 0b00000100){
			putpixel(ccr_x+5,ccr_y+y,10);
		}
		if(mouseline & 0b00000010){
			putpixel(ccr_x+6,ccr_y+y,10);
		}
		if(mouseline & 0b00000001){
			putpixel(ccr_x+7,ccr_y+y,10);
		}
	}

	oldx = ccr_x;
	oldy = ccr_y;
	// EOI
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

unsigned char kbdus[128] ={
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    VK_UP,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    VK_LEFT,	/* Left Arrow */
    0,
    VK_RIGHT,	/* Right Arrow */
  '+',
    1,	/* 79 - End key*/
    VK_DOWN,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};	

unsigned volatile char keyword = 0;
void irq_keyboard(){
	unsigned volatile char x = inportb(PS2_DATA);
	if((x&0x80)==0){
		unsigned volatile char tkeyword = kbdus[x];
		keyword = tkeyword;
		//((unsigned char*)keyword)[0] = tkeyword;
	}
	outportb(0x20,0x20);
}

int init_ps2_keyboard(){

	if(!writeToFirstPS2Port(0xF4)){goto error;}
	if(!waitforps2ok()){goto error;}
	
    	setNormalInt(1,(unsigned long)keyboardirq);
    	return 1;
    	
    	error:
    	return 0;
}

void mousewaita(){
	unsigned long timeout = 100000;
	while(--timeout){
		if((inportb(0x64) & 0x01)==1){
			return;
		}
	}
}

void mousewaitb(){
	unsigned long timeout = 100000;
	while(--timeout){
		if(!(inportb(0x64) & 0x02)){
			return;
		}
	}
}

// found info at https://github.com/stevej/osdev/blob/master/kernel/devices/mouse.c
int init_ps2_mouse(){
	mousewaita();
	outportb(0x64,0xA8);
	mousewaita();
	outportb(0x64,0x20);
	mousewaitb();
	unsigned char status =  inportb(0x60);
	inportb(0x60);
	status |= (1 << 1);
	status &= ~(1 << 5);
	mousewaita();
	outportb(0x64,0x60);
	mousewaita();
	outportb(0x60,status);
	
//	if(!writeToSecondPS2Port(0xFF)){goto error;}
//	resetTicks();
//	while(inportb(PS2_DATA)!=0xAA){
//		if(getTicks()==PS2_TIMEOUT){
//			goto error;
//		}
//	}
//	if(!writeToSecondPS2Port(0xF2)){goto error;}
//	if(!waitforps2ok()){goto error;}
//	resetTicks();
//	while(getPS2ReadyToRead()==0){
//		if(getTicks()==PS2_TIMEOUT){
//			goto error;
//		}
//	}
//	unsigned char c = inportb(PS2_DATA);
//	printps2devicetype(c);
	if(!writeToSecondPS2Port(0xF6)){goto error;}
	if(!waitforps2ok()){goto error;}
	if(!writeToSecondPS2Port(0xF4)){goto error;}
	if(!waitforps2ok()){goto error;}
	
    	setNormalInt(12,(unsigned long)mouseirq);
    	return 1;
    	
    	error:
    	return 0;
}

char ps2onceagain = 1;

void init_ps2(){
	int t = ps2_echo();
	if(t==0){
		printstring("<<PS2ECHO FAILED>>");
		for(;;);
	}
	if(getGrubStatus().keyboard==0){
		if(init_ps2_mouse()){
			printstring("PS2: mouse enabled!\n");
		}else{
			printstring("PS2: mouse disabled!\n");
			for(;;);
		}
	}
	if(init_ps2_keyboard()){
		printstring("PS2: keyboard enabled!\n");
	}else{
		printstring("PS2: keyboard disabled!\n");
		for(;;);
	}
}

InputStatus getInputStatus(){
	InputStatus is;
	is.mouse_x		= ccr_x;
	is.mouse_y		= ccr_y;
	is.mouse_z		= 0xCD;
	is.mousePressed		= clck;
	unsigned long tx = usb_get_keyboard();
	if(tx==0){
		is.keyPressed		= keyword;
	}else{
		is.keyPressed		= get_usb_hid_keyboard_input((USB_DEVICE*)tx,0);
	}
	keyword = 0x00;
	clck	= 0x00;
	return is;
}

extern char keywait();

unsigned char getch(){
	unsigned long tx = usb_get_keyboard();
	if(tx==0){
		return keywait();
	}else{
		return get_usb_hid_keyboard_input((USB_DEVICE*)tx,1);
	}
}
