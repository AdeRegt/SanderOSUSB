#include "../kernel.h"
unsigned char* videomemory = (unsigned char*)0xb8000;

typedef struct{
	unsigned char isActive;
	unsigned char isDrawable;
	unsigned char isSelected;
	unsigned char hasFocus;
	unsigned long draw;
	unsigned short x;
	unsigned short y;
	unsigned short w;
	unsigned short h;
	unsigned long value;
	unsigned long onSelected;
	unsigned long onFocus;
	unsigned char isController;
}GUIControlObject;

#define MAXGUIOBJ 20
GUIControlObject guiobjects[MAXGUIOBJ];
int selectId = 0;

void draw(){
	cls();
	for(int i = 0 ;  i < MAXGUIOBJ ; i++){
		if(guiobjects[i].isActive==1){
			if(guiobjects[i].isDrawable==1){
				if(guiobjects[i].draw!=0){
					void (*foo)(GUIControlObject o) = (void *)guiobjects[i].draw;
					foo(guiobjects[i]);
				}
			}
		}
	}
}

void unfocus(){
	for(int i = 0 ;  i < MAXGUIOBJ ; i++){
		if(guiobjects[i].isActive==1){
			if(guiobjects[i].isDrawable==1){
				if(guiobjects[i].isController==1){
					if(guiobjects[i].isSelected){
						guiobjects[i].isSelected = 0;
					}
				}
			}
		}
	}
}

void previousFocus(){
	for(int i = 0 ;  i < MAXGUIOBJ ; i++){
		if(guiobjects[i].isActive==1){
			if(guiobjects[i].isDrawable==1){
				if(guiobjects[i].isController==1){
					if(guiobjects[i].isSelected){
						guiobjects[i].isSelected = 0;
						i--;
						for(int y = i ;  y >-1 ; y--){
							if(guiobjects[y].isActive==1){
								if(guiobjects[y].isDrawable==1){
									if(guiobjects[y].isController==1){
										guiobjects[y].isSelected = 1;
										return;
									}
								}
							}
						}
						i++;
						guiobjects[i].isSelected = 1;
					}
				}
			}
		}
	}
}

void nextFocus(){
	for(int i = 0 ;  i < MAXGUIOBJ ; i++){
		if(guiobjects[i].isActive==1){
			if(guiobjects[i].isDrawable==1){
				if(guiobjects[i].isController==1){
					if(guiobjects[i].isSelected){
						guiobjects[i].isSelected = 0;
						i++;
						for(int y = i ;  y < MAXGUIOBJ ; y++){
							if(guiobjects[y].isActive==1){
								if(guiobjects[y].isDrawable==1){
									if(guiobjects[y].isController==1){
										guiobjects[y].isSelected = 1;
										return;
									}
								}
							}
						}
						i--;
						guiobjects[i].isSelected = 1;
					}
				}
			}
		}
	}
}

unsigned long getSelectedItem(){
	for(int i = 0 ;  i < MAXGUIOBJ ; i++){
		if(guiobjects[i].isActive==1){
			if(guiobjects[i].isDrawable==1){
				if(guiobjects[i].isController==1){
					if(guiobjects[i].isSelected){
						return guiobjects[i].value;
					}
				}
			}
		}
	}
	return 0;
}

unsigned long show(){
	// alleen 1 aantekenen
	for(int i = 0 ;  i < MAXGUIOBJ ; i++){
		if(guiobjects[i].isActive==1){
			if(guiobjects[i].isDrawable==1){
				if(guiobjects[i].isController==1){
					guiobjects[i].isSelected = 1;
					break;
				}
			}
		}
	}
	// leeg maken
	draw();
	while(1){
		InputStatus IS = getInputStatus();
		if(IS.keyPressed){
			if(IS.keyPressed=='\n'){
				return getSelectedItem();
			}else if(IS.keyPressed==VK_LEFT||IS.keyPressed==VK_UP){
				previousFocus();
			}else if(IS.keyPressed==VK_RIGHT||IS.keyPressed==VK_DOWN){
				nextFocus();
			}
			draw();
		}
		if(IS.mousePressed){
			for(int i = 0 ;  i < MAXGUIOBJ ; i++){
				if(guiobjects[i].isActive==1){
					if(guiobjects[i].isDrawable==1){
						if(guiobjects[i].isController==1){
							int aX = guiobjects[i].x;
							int aY = guiobjects[i].y;
							int bX = aX + guiobjects[i].w;
							int bY = aY + guiobjects[i].h;
							if((aX<IS.mouse_x&&bX>IS.mouse_x)&&(aY<IS.mouse_y&&bY>IS.mouse_y)){
								if(guiobjects[i].isSelected){
									return getSelectedItem();
								}else{
									unfocus();
									guiobjects[i].isSelected = 1;
									draw();
								}
							}
						}
					}
				}
			}
			
		}
		resetTicks();
		while(getTicks()!=2){}
	}
}

void drawRect(GUIControlObject o){
	for(int x = 0 ; x < o.w ; x++){
		for(int y = 0 ; y < o.h ; y++){
			putpixel(o.x+x,o.y+y,0x04);
		}
	}
}

void drawString(GUIControlObject o){
	unsigned char* msg = (unsigned char*) o.value;
	unsigned char deze;
	int i = 0;
	int i2 = 0;
	while((deze = msg[i++])!=0x00){
		if((o.x+((i2+1)*8))>(o.x+o.w)){
			o.y += 8;
			i2 = 0;
		}
		if(deze!=' '){
			drawcharraw(deze,o.x+(i2*8),o.y,0x06,0x04);
		}
		i2++;
	}
}

void drawButton(GUIControlObject o){
	unsigned char* msg = (unsigned char*) o.value;
	unsigned char deze;
	int i = 0;
	int i2 = 0;
	for(int x = 0 ; x < o.w ; x++){
		for(int y = 0 ; y < o.h ; y++){
			putpixel(o.x+x,o.y+y,o.isSelected?0x50:0x10);
		}
	}
	
	while((deze = msg[i++])!=0x00){
		if((o.x+2+((i2+1)*8))>(o.x+o.w)){
			o.y += 8;
			i2 = 0;
		}
		if(deze!=' '){
			drawcharraw(deze,o.x+2+(i2*8),o.y+2,0x06,o.isSelected?0x50:0x10);//4
		}
		i2++;
	}
}

void message(char *message){
	char *okmessage = "OK";
	freeGui();
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,100,280,100,(unsigned long)message,0,0,0);
	addController(1,(unsigned long)&drawButton,50,150,50,15,(unsigned long)okmessage,0,0,1);
	show();
}

char confirm(char *message){
	char *okmessage = "OK";
	char *cancelmessage = "CANCEL";
	freeGui();
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,100,280,100,(unsigned long)message,0,0,0);
	addController(1,(unsigned long)&drawButton,50,150,50,15,(unsigned long)okmessage,0,0,1);
	addController(1,(unsigned long)&drawButton,110,150,50,15,(unsigned long)cancelmessage,0,0,1);
	return memcmp((char*)show(),(char*)okmessage,2)==0;
}

char *browse(){
	char *result = malloc(200);
	int pnt = 0;
	char *sigma = (char *)browseDIR("@");
	result[pnt++] = sigma[0];
	result[pnt++] = '@';
	while(1){
		char *taf = (char *)browseDIR(result);
		if(pnt!=2){
			result[pnt++] = '/';
		}
		int z = 0;
		int y = 0;
		while(1){
			char deze = taf[z++];
			if(deze=='.'){
				y = 1;
			}
			if(deze==0x00){
				break;
			}
			result[pnt++] = deze;
		}
		if(y){
			break;
		}
	}
	
	return result;
}

char *browseDIR(char *path){
	freeGui();
	char *message = "PLEASE PICK A FILE";
	char *filesystemtext = dir(path);
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,100,280,100,(unsigned long)message,0,0,0);
	addController(1,(unsigned long)&drawString,20,120,280,100,(unsigned long)path,0,0,0);
	int i = 0;
	int t = 22;
	int r = 140;
	while(1){
		char d = filesystemtext[i];
		if(d==0x00){
			break;
		}
		char *cont = malloc(20);
		int z = 0;
		while(1){
			char e = filesystemtext[i++];
			if(e==0x00){
				i--;
				break;
			}
			if(e==';'){
				break;
			}
			cont[z++] = e;
		}
		addController(1,(unsigned long)&drawButton,t,r,50,15,(unsigned long)cont,0,0,1);
		t += 55;
		if(t>300){
			t = 22;
			r+= 20;
		}
	}
	return (char *)show();
}

void freeGui(){
	for(int i = 0 ; i < MAXGUIOBJ ; i++){
		guiobjects[i].isActive = 0;
	}
	selectId = 0;
}

int getFreeGui(){
	return selectId;
}

void addController(unsigned char drawable,unsigned long drawablefunc,unsigned short x,unsigned short y,unsigned short w,unsigned short h,unsigned long value,unsigned long onSelected,unsigned long onFocus,unsigned char controller){
	if(selectId>=MAXGUIOBJ){
		return;
	}
	guiobjects[selectId].isActive = 1;
	guiobjects[selectId].isDrawable = drawable;
	guiobjects[selectId].isSelected = 0;
	guiobjects[selectId].hasFocus = 0;
	guiobjects[selectId].draw = drawablefunc;
	guiobjects[selectId].x = x;
	guiobjects[selectId].y = y;
	guiobjects[selectId].w = w;
	guiobjects[selectId].h = h;
	guiobjects[selectId].value = value;
	guiobjects[selectId].onSelected = onSelected;
	guiobjects[selectId].onFocus = onFocus;
	guiobjects[selectId].isController = controller;
	selectId++;
}

//
// STRING
//
//

extern void video_load_font();

int vidpnt = 0;
int curx = 0;
int cury = 0;
unsigned char _fontbuffer[4000];
int isgraphics = 0;

int isGraphicsMode(){
	return isgraphics;
}

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
	isgraphics = 0;
	video_load_font();
}

void printstring(char* message){
	int a = 0;
	char b = 0;
	while((b=message[a++])!=0x00){
		putc(b);
	}
}

unsigned char font[4000] = {

//
// 0
0b01111111,
0b01100001,
0b01010001,
0b01001001,
0b01000101,
0b01111111,

//
// 1
0b00001000,
0b00011000,
0b00001000,
0b00001000,
0b00001000,
0b01111111,

//
// 2
0b01111100,
0b01000010,
0b00000100,
0b00011000,
0b00100000,
0b01111111,

//
// 3
0b01111111,
0b00000001,
0b00001111,
0b00000001,
0b00000001,
0b01111111,

//
// 4
0b01000100,
0b01000100,
0b01111110,
0b00000100,
0b00000100,
0b00000100,

//
// 5
0b01111111,
0b01000000,
0b01000000,
0b01111111,
0b00000001,
0b01111111,

//
// 6
0b00000001,
0b00001000,
0b00100000,
0b01111111,
0b01000001,
0b01111111,

//
// 7
0b01111111,
0b00000001,
0b00000010,
0b00000100,
0b00001000,
0b00010000,

//
// 8
0b00010000,
0b01000100,
0b00011000,
0b01000100,
0b00101000,
0b00010000,

//
// 9
0b01111111,
0b01000001,
0b01111111,
0b00000010,
0b00001000,
0b00100000,

//
// :
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// ;
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// <
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// =
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// >
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// ?
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// @
0b01111111,
0b01000001,
0b01011001,
0b01011111,
0b01000000,
0b01111111,

//
// A
0b00011000,
0b00100100,
0b01000010,
0b01111110,
0b01000010,
0b01000010,

//
// B
0b01111110,
0b01000001,
0b01000010,
0b01111100,
0b01000010,
0b01111110,

//
// C
0b01111111,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01111111,

//
// D
0b01111100,
0b01000010,
0b01000001,
0b01000010,
0b01000100,
0b01111000,

//
// E
0b01111111,
0b01000000,
0b01110000,
0b01000000,
0b01000000,
0b01111111,

//
// F
0b01111111,
0b01000000,
0b01110000,
0b01000000,
0b01000000,
0b01000000,

//
// G
0b01111111,
0b01000000,
0b01000000,
0b01001111,
0b01000001,
0b01111111,

//
// H
0b01000001,
0b01000001,
0b01111111,
0b01000001,
0b01000001,
0b01000001,

//
// I
0b00010000,
0b00000000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,

//
// J
0b00000001,
0b00000001,
0b00000001,
0b00000001,
0b00000001,
0b01111111,

//
// K
0b01000001,
0b01000010,
0b01111100,
0b01000100,
0b01000010,
0b01000001,

//
// L
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01111111,

//
// M
0b01000001,
0b01100011,
0b01010101,
0b01001001,
0b01000001,
0b01000001,

//
// N
0b01000001,
0b01100001,
0b01010001,
0b01000101,
0b01000011,
0b01000001,

//
// O
0b01111111,
0b01000001,
0b01000001,
0b01000001,
0b01000001,
0b01111111,

//
// P
0b01111111,
0b01000001,
0b01111111,
0b01000000,
0b01000000,
0b01000000,

//
// Q
0b01111111,
0b01000001,
0b01000001,
0b01000101,
0b01000011,
0b01111111,

//
// R
0b01111111,
0b01000001,
0b01111111,
0b01000100,
0b01000010,
0b01000001,

//
// S
0b01111111,
0b01000000,
0b01000000,
0b01111111,
0b00000001,
0b01111111,

//
// T
0b01111111,
0b00001000,
0b00001000,
0b00001000,
0b00001000,
0b00001000,

//
// U
0b01000001,
0b01000001,
0b01000001,
0b01000001,
0b01000001,
0b01111111,

//
// V
0b01000001,
0b00100010,
0b00100010,
0b00010100,
0b00010100,
0b00001000,

//
// W
0b01000001,
0b01000001,
0b01000001,
0b01001001,
0b01001001,
0b01111111,

//
// X
0b00100001,
0b00010010,
0b00001100,
0b00001100,
0b00010010,
0b00100001,

//
// Y
0b00100001,
0b00010010,
0b00001100,
0b00001100,
0b00001100,
0b00001100,

//
// Z
0b01111111,
0b00000010,
0b00000100,
0b00011000,
0b00100000,
0b01111111,

//
// breath space

0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,

//
// A
0b00011000,
0b00100100,
0b01000010,
0b01111110,
0b01000010,
0b01000010,

//
// B
0b01111110,
0b01000001,
0b01000010,
0b01111100,
0b01000010,
0b01111110,

//
// C
0b01111111,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01111111,

//
// D
0b01111100,
0b01000010,
0b01000001,
0b01000010,
0b01000100,
0b01111000,

//
// E
0b01111111,
0b01000000,
0b01110000,
0b01000000,
0b01000000,
0b01111111,

//
// F
0b01111111,
0b01000000,
0b01110000,
0b01000000,
0b01000000,
0b01000000,

//
// G
0b01111111,
0b01000000,
0b01000000,
0b01001111,
0b01000001,
0b01111111,

//
// H
0b01000001,
0b01000001,
0b01111111,
0b01000001,
0b01000001,
0b01000001,

//
// I
0b00010000,
0b00000000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,

//
// J
0b00000001,
0b00000001,
0b00000001,
0b00000001,
0b00000001,
0b01111111,

//
// K
0b01000001,
0b01000010,
0b01111100,
0b01000100,
0b01000010,
0b01000001,

//
// L
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01111111,

//
// M
0b01000001,
0b01100011,
0b01010101,
0b01001001,
0b01000001,
0b01000001,

//
// N
0b01000001,
0b01100001,
0b01010001,
0b01000101,
0b01000011,
0b01000001,

//
// O
0b01111111,
0b01000001,
0b01000001,
0b01000001,
0b01000001,
0b01111111,

//
// P
0b01111111,
0b01000001,
0b01111111,
0b01000000,
0b01000000,
0b01000000,

//
// Q
0b01111111,
0b01000001,
0b01000001,
0b01000101,
0b01000011,
0b01111111,

//
// R
0b01111111,
0b01000001,
0b01111111,
0b01000100,
0b01000010,
0b01000001,

//
// S
0b01111111,
0b01000000,
0b01000000,
0b01111111,
0b00000001,
0b01111111,

//
// T
0b01111111,
0b00001000,
0b00001000,
0b00001000,
0b00001000,
0b00001000,

//
// U
0b01000001,
0b01000001,
0b01000001,
0b01000001,
0b01000001,
0b01111111,

//
// V
0b01000001,
0b00100010,
0b00100010,
0b00010100,
0b00010100,
0b00001000,

//
// W
0b01000001,
0b01000001,
0b01000001,
0b01001001,
0b01001001,
0b01111111,

//
// X
0b00100001,
0b00010010,
0b00001100,
0b00001100,
0b00010010,
0b00100001,

//
// Y
0b00100001,
0b00010010,
0b00001100,
0b00001100,
0b00001100,
0b00001100,

//
// Z
0b01111111,
0b00000010,
0b00000100,
0b00011000,
0b00100000,
0b01111111,
};

void drawcharraw(unsigned char c, int offsetX, int offsetY, int fgcolor, int bgcolor)
{
	
	int selector = (c-'0')*6;
	for(int i = 0 ; i < 6 ; i++){
		if(font[selector+i] & 0b10000000){
			putpixel(offsetX+0,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+0,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b01000000){
			putpixel(offsetX+1,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+1,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b00100000){
			putpixel(offsetX+2,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+2,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b00010000){
			putpixel(offsetX+3,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+3,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b00001000){
			putpixel(offsetX+4,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+4,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b00000100){
			putpixel(offsetX+5,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+5,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b00000010){
			putpixel(offsetX+6,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+6,offsetY+i,bgcolor);
		}
		if(font[selector+i] & 0b00000001){
			putpixel(offsetX+7,offsetY+i,fgcolor);
		}else{
			putpixel(offsetX+7,offsetY+i,bgcolor);
		}
	}
}

void drawchar(unsigned char c, int x, int y, int fgcolor, int bgcolor)
{
	int offsetX = x*8;
	int offsetY = y*7;
	drawcharraw(c,offsetX,offsetY,fgcolor,bgcolor);
	
}

void putc(char a){
	if(isgraphics==1){
		if(curx==40||a=='\n'){
			curx=0;
			cury++;
		}else if(a==' '){
			curx++;
		}else{
			drawchar(a,curx,cury,0x04,0x01);
			curx++;
		}
	}else{
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
    	unsigned where = x + (y*320);
    	screen[where] = color;
}

char getpixel(int x,int y){
	unsigned char* screen = (unsigned char*)0xA0000;
    	unsigned where = x + (y*320);
    	return screen[where];
}

void cls(){
	for(int x = 0 ; x < 320 ; x++){
		for(int y = 0 ; y < 200 ; y++){
			putpixel(x,y,0x01);
		}
	}
	
	curx = 0;
	cury = 0;
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
   unsigned int a;

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
	isgraphics = 1;
   return 1;

}

