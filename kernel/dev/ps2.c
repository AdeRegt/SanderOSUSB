#include "../kernel.h"

//
// PS2
//
//

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64
#define PS2_TIMEOUT 10

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
	outportb(PS2_COMMAND,0xD4);
	resetTicks();
	while(getPS2ReadyToWrite()>0){
		if(getTicks()==PS2_TIMEOUT){return 0;}
	}
	outportb(PS2_DATA,data);
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

void printps2devicetype(unsigned char a){
	if(a==0x00){
		printstring("PS2: standard ps/2 mouse\n");
	}else if(a==0x03){
		printstring("PS2: mouse with scroll wheel\n");
	}else if(a==0x04){
		printstring("PS2: 5 button mouse\n");
	}else if(a==0xab||a==0x41||a==0xab||a==0xc1||a==0x83){
		printstring("PS2: keyboard\n");
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
void irq_mouse(){
	if(csr_t==0){
		char A = inportb(PS2_DATA);
		if(ccr_b){
			if((ccr_y+A)<200){
				ccr_y += A;
			}
		}else{
			if((ccr_y-A)>0){
				ccr_y -= A;
			}
		}
		csr_t = 1;
	}else if(csr_t==1){
		char A = inportb(PS2_DATA);
		if((A & 0b00000001)>0){
			printstring("_LEFT");
		}
		if((A & 0b00000010)>0){
			printstring("_RIGHT");
		}
		if((A & 0b00000100)>0){
			printstring("_MIDDLE");
			ccr_x = 50;
			ccr_y = 50;
			csr_y = 12;
			csr_x = 40;
		}
		if((A & 0b00001000)>0){
			ccr_a = 1;
		}else{
			ccr_a = 0;
		}
		if((A & 0b00010000)>0){
			ccr_b = 1;
		}else{
			ccr_b = 0;
		}
		csr_t = 2;
	}else if(csr_t==2){
		char A = inportb(PS2_DATA);
		if(ccr_a){
			if((ccr_x+A)<1600){
				ccr_x += A;
			}
		}else{
			if((ccr_x-A)>0){
				ccr_x -= A;
			}
		}
		csr_t = 0;
	}
	
	if(isGraphicsMode()){
		putpixel(oldx,oldy,getpixel(oldx,oldy));
		putpixel(ccr_x,ccr_y,2);
		oldx = ccr_x;
		oldy = ccr_y;
	}else{
		// hardware cursor updaten
		unsigned temp;
		csr_x = ccr_x/20;
		csr_y = ccr_y/20;
		if(csr_x>75){
			csr_x = 70;
		}
		if(csr_y>24){
			csr_y = 20;
		}
	    	temp = csr_y * 80 + csr_x;
	    	outportb(0x3D4, 14);
	    	outportb(0x3D5, temp >> 8);
	    	outportb(0x3D4, 15);
	    	outportb(0x3D5, temp);
	}
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
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
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
	
	// detectie
	if(!writeToFirstPS2Port(0xF5)){goto error;}
	if(!waitforps2ok()){goto error;}
	if(!writeToFirstPS2Port(0xF2)){goto error;}
	if(!waitforps2ok()){goto error;}
	resetTicks();
	while(getPS2ReadyToRead()==0){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
	unsigned char a = inportb(PS2_DATA);
	resetTicks();
	while(getPS2ReadyToRead()==0){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
	unsigned char b = inportb(PS2_DATA);
	printps2devicetype(a);
	printps2devicetype(b);
	
	if(!writeToFirstPS2Port(0xFF)){goto error;}
	resetTicks();
	while(inportb(PS2_DATA)!=0xAA){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
	if(!writeToFirstPS2Port(0xF6)){goto error;}
	if(!waitforps2ok()){goto error;}
	if(!writeToFirstPS2Port(0xF4)){goto error;}
	if(!waitforps2ok()){goto error;}
	
    	setNormalInt(1,(unsigned long)keyboardirq);
    	return 1;
    	
    	error:
    	return 0;
}

int init_ps2_mouse(){
	
	// detectie
	if(!writeToSecondPS2Port(0xFF)){goto error;}
	resetTicks();
	while(inportb(PS2_DATA)!=0xAA){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
	if(!writeToSecondPS2Port(0xF5)){goto error;}
	if(!waitforps2ok()){goto error;}
	if(!writeToSecondPS2Port(0xF2)){goto error;}
	if(!waitforps2ok()){goto error;}
	resetTicks();
	while(getPS2ReadyToRead()==0){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
	unsigned char c = inportb(PS2_DATA);
	resetTicks();
	while(getPS2ReadyToRead()==0){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
	unsigned char d = inportb(PS2_DATA);
	printps2devicetype(c);
	printps2devicetype(d);
	
	if(!writeToSecondPS2Port(0xFF)){goto error;}
	resetTicks();
	while(inportb(PS2_DATA)!=0xAA){
		if(getTicks()==PS2_TIMEOUT){
			goto error;
		}
	}
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
	char ps2status = getPS2StatusRegisterText();
	if((ps2status & 0b00000001)>0){
		printstring("PS2: read buffer full\n");
	}
	if((ps2status & 0b00000010)>0){
		printstring("PS2: write buffer full\n");
	}
	if((ps2status & 0b00000100)>0){
		printstring("PS2: passed selftest\n");
	}
	if((ps2status & 0b00001000)>0){
		printstring("PS2: data for controller\n");
	}else{
		printstring("PS2: data for device\n");
	}
	while(getPS2ReadyToWrite()!=0){}
	outportb(PS2_COMMAND,0x20);
	while(getPS2ReadyToRead()==0){
		if(getTicks()>=PS2_TIMEOUT){
			printstring("__TIMEOUT__\n");
			break;
		}
	}
	char ps2enable = inportb(PS2_DATA);
	if((ps2enable & 0b00000001)>0){
		printstring("PS2: port1 interrupt enabled\n");
	}
	if((ps2enable & 0b00000010)>0){
		printstring("PS2: port2 interrupt enabled\n");
	}
	if((ps2enable & 0b00000100)>0){
		printstring("PS2: passed POST\n");
	}
	if((ps2enable & 0b00010000)>0){
		printstring("PS2: port1 clock enabled\n");
	}
	if((ps2enable & 0b00100000)>0){
		printstring("PS2: port2 clock enabled\n");
	}
	if((ps2enable & 0b01000000)>0){
		printstring("PS2: porttranslation enabled\n");
	}
	//while(getPS2ReadyToWrite()!=0){}
	//outportb(PS2_COMMAND,0xAE);
	//while(getPS2ReadyToRead()==0){
	//	if(getTicks()>=10){
	//		printstring("__TIMEOUT__\n");
	//		break;
	//	}
	//}
	//while(getPS2ReadyToWrite()!=0){}
	//outportb(PS2_COMMAND,0xA8);
	//while(getPS2ReadyToRead()==0){
	//	if(getTicks()>=10){
	//		printstring("__TIMEOUT__\n");
	//		break;
	//	}
	//}
	if(init_ps2_keyboard()){
		printstring("PS2: keyboard enabled!\n");
	}else{
		printstring("PS2: keyboard disabled!\n");
		for(;;);
	}
	if(ps2onceagain){
		if(init_ps2_mouse()){
			printstring("PS2: mouse enabled!\n");
		}else{
			printstring("PS2: mouse disabled!\n");
			ps2onceagain = 0;
			init_ps2();
		}
	}
	
    	
}

extern char keywait();

unsigned char getch(){
	return keywait();
}
