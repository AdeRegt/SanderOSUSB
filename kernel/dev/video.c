#include "../kernel.h"
unsigned char* videomemory = (unsigned char*)0xb8000;

static unsigned int SCREEN_WIDTH = 0;
static unsigned int SCREEN_HEIGHT = 0;

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

#define MAXGUIOBJ 100
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
	force_mouse_resample();
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
				if(guiobjects[i].isController==1||guiobjects[i].isController==2){
					if(guiobjects[i].isSelected){
						guiobjects[i].isSelected = 0;
						i--;
						for(int y = i ;  y >-1 ; y--){
							if(guiobjects[y].isActive==1){
								if(guiobjects[y].isDrawable==1){
									if(guiobjects[y].isController==1||guiobjects[y].isController==2){
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
				if(guiobjects[i].isController==1||guiobjects[i].isController==2){
					if(guiobjects[i].isSelected){
						guiobjects[i].isSelected = 0;
						i++;
						for(int y = i ;  y < MAXGUIOBJ ; y++){
							if(guiobjects[y].isActive==1){
								if(guiobjects[y].isDrawable==1){
									if(guiobjects[y].isController==1||guiobjects[y].isController==2){
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
				if(guiobjects[i].isController==1||guiobjects[i].isController==2){
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
				if(guiobjects[i].isController==1||guiobjects[i].isController==2){
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
			}else{
				// is it a selectable?
				for(int i = 0 ;  i < MAXGUIOBJ ; i++){
					if(guiobjects[i].isActive==1){
						if(guiobjects[i].isDrawable==1){
							if(guiobjects[i].isController==2&&guiobjects[i].isSelected==1){
								unsigned char *t = (unsigned char*)guiobjects[i].value;
								int z = strlen((char*)t);
								if(IS.keyPressed=='\b'){
									t[z-1] = 0;
								}else if(z<(guiobjects[i].w/8)){
									t[z] = IS.keyPressed;
								}
							}
						}
					}
				}
			}
			draw();
		}
		if(IS.mousePressed){
			for(int i = 0 ;  i < MAXGUIOBJ ; i++){
				if(guiobjects[i].isActive==1){
					if(guiobjects[i].isDrawable==1){
						if(guiobjects[i].isController==1||guiobjects[i].isController==2){
							int aX = guiobjects[i].x;
							int aY = guiobjects[i].y;
							int bX = aX + guiobjects[i].w;
							int bY = aY + guiobjects[i].h;
							if((aX<IS.mouse_x&&bX>IS.mouse_x)&&(aY<IS.mouse_y&&bY>IS.mouse_y)){
								if(guiobjects[i].isSelected&&guiobjects[i].isController==1){
									return getSelectedItem();
								}else{
									unfocus();
									guiobjects[i].isSelected = 1;
									draw();
								}
							}else{
								guiobjects[i].isSelected = 0;
							}
						}
					}
				}
			}
			
		}
		resetTicks();
		while(getTicks()!=1){}
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
			drawcharraw(deze,
			o.x + 2 + (i2 * 8),
			o.y + (o.h - 6) * 0.5,
			0x06,
			o.isSelected ? 0x50 : 0x10);//4
		}
		i2++;
	}
}

typedef struct {
	unsigned int size;
	unsigned int reserved[4];
	unsigned int pixelBufferOffset;
	char bmptype[2];
} BMP_HEADER;

typedef struct {
	unsigned int size;
	unsigned int width;
	unsigned int height;
} BMP_DBIHEADER;

/* List of compression methods we should aim to support */
enum BMPCompressionMethod {
	BI_RGB,
	BI_RLE8,
	BI_RLE4,
	BI_JPEG
};

/*
 * Parse a bmp file and return the pixel buffer.
 * For info about the file structure refere to:
 * https://en.wikipedia.org/wiki/BMP_file_format#File_structure
 */
unsigned char* getImageFromBMP(unsigned char* file_buffer, unsigned int* width, unsigned int* height) {
	BMP_HEADER header = *(BMP_HEADER*)file_buffer;
	*width = *(unsigned short*)(file_buffer + 0x12);
	*height = *(unsigned short*)(file_buffer + 0x16);

	if (header.bmptype[0] == 'B' && header.bmptype[0] == 'M') {
		//unsigned short bits_per_pixel = *(file_buffer + 0x1C);
		unsigned int compression_method = *(file_buffer + 0x1E);
		if (compression_method != BI_RGB && compression_method != BI_RLE4) {
			return NULL;
		}
	}

	return file_buffer + header.pixelBufferOffset;
}

/*
 * Renders a bmp file into the screen.
 * NOTE: For now just render the entire image in the screen
 * (should be the same size as the window)
*/
void draw_bmp(unsigned char* file_buffer, unsigned short offsetX, unsigned short offsetY) {
	// read file_buffer and extract the pixel_buffer
	unsigned int width, height;
	unsigned char* pixel_buffer = getImageFromBMP(file_buffer, &width, &height);

	// get the bounds of the image
	unsigned int draw_w = (((unsigned int)offsetX + width) >= SCREEN_WIDTH) ? SCREEN_WIDTH : width;
	unsigned int draw_h = (((unsigned int)offsetY + height) >= SCREEN_HEIGHT) ? SCREEN_HEIGHT : height;

	// draw the image
	for(unsigned int x = 0 ; x < draw_w ; x++){
		for(unsigned int y = 0 ; y < draw_h ; y++){
			putpixel(offsetX + x, offsetY + y, pixel_buffer[x + width * y]);
		}
	}
}

void drawInputBox(GUIControlObject o){
	unsigned char background_color = 0x07;
	unsigned char foreground_color = 0x00;
	for(int x = 0 ; x < o.w ; x++){
		for(int y = 0 ; y < o.h ; y++){
			putpixel(o.x+x,o.y+y,background_color);
		}
	}
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
			drawcharraw(deze,o.x+(i2*8)+1,o.y+1,foreground_color,background_color);
		}
		i2++;
	}
	if(o.isSelected==0x01){
		drawcharraw('_',o.x+(i2*8)+1,o.y+1,foreground_color,background_color);
	}
}

unsigned char* prompt(char *message,int maxinput){
	char *okmessage = "OK";
	unsigned char * result = (unsigned char*)malloc(maxinput);
	freeGui();
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,80,280,100,(unsigned long)message,0,0,0);
	addController(1,(unsigned long)&drawInputBox,20,100,maxinput*8,10,(unsigned long)result,0,0,2);
	addController(1,(unsigned long)&drawButton,50,150,50,15,(unsigned long)okmessage,0,0,1);
	show();
	return result;
}

void message(char *message){
	char *okmessage = "OK";
	freeGui();
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,100,280,100,(unsigned long)message,0,0,0);
	addController(1,(unsigned long)&drawButton,50,150,50,15,(unsigned long)okmessage,0,0,1);
	show();
}

char choose(char *message,int argcount,char **args){
	freeGui();
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,100,280,100,(unsigned long)message,0,0,0);
	for(int i = 0 ; i < argcount ; i++){
		char *msg = (char*)args[i];
		addController(1,(unsigned long)&drawButton,50+(i*60),150,50,15,(unsigned long)msg,0,0,1);
	}
	char* res = (char*)show();
	int tok = 0;
	for(int i = 0 ; i < argcount ; i++){
		char *msg = (char*)args[i];
		if(memcmp((char*)res,(char*)msg,strlen(msg))==0){
			tok = i;
		}
	}
	return tok;
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
	char *result = (char *)malloc(200);
	again:
	for(int i = 0 ; i < 200 ; i++){
		result[i] = 0x00;
	}
	int pnt = 0;
	char *sigma = (char *)browseDIR("@");
	if(sigma==0||sigma[0]==0){
		return 0;
	}
	if(sigma[0]=='@'||sigma[0]=='^'){
		goto again;
	}
	result[pnt++] = sigma[0];
	result[pnt++] = '@';
	result[pnt] = 0;
	char *taf;
	while(1){
		pt:
		taf = (char *)browseDIR(result);

		if(taf==0||taf[0]==0){
			message("Directory does not exists");
			result[0] = '@';
			result[1] = '\0';
			result[2] = '\0';
			pnt = 0;
			goto again;
		}
		if(taf[0]=='^'){
			// one DIR up
			printf("In %s %x \n",result,pnt);
			nogeens:
			if(!(result[pnt]=='/'||result[pnt]=='@')){
				result[pnt] = 0x00;
				pnt--;
				goto nogeens;
			}
			if(result[pnt]=='/'){
				result[pnt]=0x00;
			}
			goto pt;
		}
		if(taf[0]=='@'){
			// to ROOT
			goto again;
		}
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
	if(filesystemtext==0){
		return "";
	}
	if(filesystemtext[0]==0x00){
		return filesystemtext;
	}
	addController(1,(unsigned long)&drawRect,10,70,300,100,0,0,0,0);
	addController(1,(unsigned long)&drawString,20,100,280,100,(unsigned long)message,0,0,0);
	addController(1,(unsigned long)&drawString,20,120,280,100,(unsigned long)path,0,0,0);
	addController(1,(unsigned long)&drawButton,50,80,25,15,(unsigned long)"@",0,0,1);
	addController(1,(unsigned long)&drawButton,80,80,25,15,(unsigned long)"^",0,0,1);
	int i = 0;
	int t = 22;
	int r = 140;
	char d = 0x00;
	while(1){
		d = filesystemtext[i];
		if(d==0x00){
			break;
		}
		int tmpx = i;
		while(1){
			char e = filesystemtext[i++];
			if(e==0x00){
				i--;
				break;
			}
			if(e==';'){
				i--;
				filesystemtext[i++] = 0;
				break;
			}
		}
		addController(1,(unsigned long)&drawButton,t,r,50,15,(unsigned long)(filesystemtext+tmpx),0,0,1);
		t += 55;
		if(t>280){
			t = 22;
			r+= 20;
		}
	}
	char *v = (char *)show();
	return v;
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
	setForeGroundBackGround(7,0);
	video_load_font();
}

void printstring(char* message){
	int a = 0;
	char b = 0;
	while((b=message[a++])!=0x00){
		putc(b);
	}
}

void print_bios_char_table(){
	printf("x: 0 1 2 3 4 5 6 7 8 9 A B C D E F \n");
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x0,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x1,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x2,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x3,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x4,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x5,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x6,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x7,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x8,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0x9,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0xA,0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0xB,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0xC,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0xD,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0xE,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF);
	printf("%x: %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c \n",0xF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF);
}

unsigned char font[4000] = {
//
// (
0b00111110,
0b01100000,
0b01000000,
0b01000000,
0b01100000,
0b00111110,

//
// )
0b01111110,
0b00000011,
0b00000001,
0b00000001,
0b00000011,
0b00111110,

//
// *
0b00000000,
0b01001000,
0b00110000,
0b00110000,
0b01001000,
0b00000000,

//
// +
0b00010000,
0b00010000,
0b01111100,
0b00010000,
0b00010000,
0b00000000,

//
// ,
0b00000000,
0b00000000,
0b00111000,
0b00111000,
0b00010000,
0b00001000,

//
// -
0b00000000,
0b00000000,
0b00000000,
0b01111110,
0b00000000,
0b00000000,

//
// .
0b00000000,
0b00000000,
0b00000000,
0b00011000,
0b00011000,
0b00000000,

//
// /
0b00000010,
0b00000100,
0b00001000,
0b00010000,
0b00100000,
0b01000000,

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
0b00111000,
0b01000100,
0b01111100,
0b01000100,
0b01000100,
0b01111100,

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

//
// [
0b01111110,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01111110,

//
// \*
0b01000000,
0b00100000,
0b00010000,
0b00001000,
0b00000100,
0b00000010,

//
// ]
0b01111110,
0b00000010,
0b00000010,
0b00000010,
0b00000010,
0b01111110,

//
// ^
0b00010000,
0b00101000,
0b01000100,
0b00000000,
0b00000000,
0b00000000,

//
// _
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b01111110,
0b00000000,

//
// `
0b00000000,
0b00111000,
0b00111000,
0b00010000,
0b00001000,
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

void drawcharraw(unsigned char c, int offsetX, int offsetY, int fgcolor, int bgcolor) {
	int selector = (c-'(')*6;
	for(int i = 0 ; i < 6 ; i++){
		unsigned char font_sector = font[selector+i];
		if(font_sector  & 0b10000000){
			putpixel(offsetX+0,offsetY+i,fgcolor);
		}
		if(font_sector & 0b01000000){
			putpixel(offsetX+1,offsetY+i,fgcolor);
		}
		if(font_sector & 0b00100000){
			putpixel(offsetX+2,offsetY+i,fgcolor);
		}
		if(font_sector & 0b00010000){
			putpixel(offsetX+3,offsetY+i,fgcolor);
		}
		if(font_sector & 0b00001000){
			putpixel(offsetX+4,offsetY+i,fgcolor);
		}
		if(font_sector & 0b00000100){
			putpixel(offsetX+5,offsetY+i,fgcolor);
		}
		if(font_sector & 0b00000010){
			putpixel(offsetX+6,offsetY+i,fgcolor);
		}
		if(font_sector & 0b00000001){
			putpixel(offsetX+7,offsetY+i,fgcolor);
		}
	}
}

void drawstringat(char* str,int x,int y,int color){
	int len = strlen(str);
	for(int i = 0 ; i < len ; i++){
		drawcharraw(str[i],x + (i*8) ,y,color,0);
	}
}

void drawchar(unsigned char c, int x, int y, int fgcolor, int bgcolor)
{
	int offsetX = x*8;
	int offsetY = y*7;
	drawcharraw(c,offsetX,offsetY,fgcolor,bgcolor);
	
}

unsigned char u_foreground = 0x04;
unsigned char u_background = 0x01;
void setForeGroundBackGround(unsigned char fg,unsigned char bg){
	u_foreground = fg;
	u_background = bg;
}

void putc(char a){
	if(isgraphics==1){
		if(cury==27){
			cls();
			curx = 0;
			cury = 1;
		}else if(curx==40||a=='\n'){
			curx=0;
			cury++;
		}else if(a==' '){
			curx++;
		}else{
			drawchar(a,curx,cury,u_foreground,u_background);
			curx++;
		}
	}else{
		if(a!='\n'){
			vidpnt = (curx*2)+(160*cury);
			videomemory[vidpnt++] = a;
			videomemory[vidpnt++] = (u_background << 4) | u_foreground;
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

/**
 * Creates a string from input
 * @param format the base string
 * @param arg the list of arguments
*/
char* vsprintf(char* format,va_list arg){
	if(strlen(format)==0){
		return NULL;
	}
	char* buffer = malloc(strlen(format));
	char* buffertmp;
	int buffersize = strlen(format) + 1;
	int oldsize;
	char *traverse; 
	unsigned int i; 
	signed int t;
	char *s;
	int travelpointer = 0;
	
	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		while( *traverse != '%' && *traverse != '\0' ) 
		{ 
			buffer[travelpointer++] = *traverse;
			traverse++; 
		} 
		if(*traverse =='\0'){
		    break; 
		}
		traverse++; 
		oldsize = buffersize;
		
		switch(*traverse) 
		{ 
			case 'c' : i = va_arg(arg,int);		//Fetch char argument
						// char is only 1 argument
						buffersize += 1;
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						if(i==0){
							buffer[travelpointer++] = '0';
						}else{
							buffer[travelpointer++] = i;
						}
						break; 
						
			case 'd' : t = va_arg(arg,int); 		//Fetch Decimal/Integer argument
						char* chachacha;
						if(t<0) 
						{ 
							t = -t;
							chachacha = "-"; 
						} 
						chachacha = convert(t,10);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(int x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 'o': i = va_arg(arg,unsigned int); //Fetch Octal representation
						chachacha = convert(i,8);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(int x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 's': s = va_arg(arg,char *); 		//Fetch string
						chachacha = s; 
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(int x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
						chachacha = convert(i,16);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(int x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
				
		}	
	}
	buffer[travelpointer++] = 0x00;
	return buffer;
}

char* sprintf(char* format,...){
	va_list arg; 
	va_start(arg, format);
	char* result = vsprintf(format,arg); 
	va_end(arg); 
	return result;
}

void debugf(char* format,...){
	va_list arg; 
	va_start(arg, format);
	char* result = vsprintf(format,arg); 
	#ifdef DEBUG_TO_SCREEN
	printstring(result);
	#endif
	if(is_virtual_box_session_enabled()){
		writer_string_vbox(result);
	}else{
		writer_string_serial(result,getDefaultSerialPort());
	}
	free(result);
	va_end(arg); 
}


void printf(char* format,...) 
{ 
	va_list arg; 
	va_start(arg, format);
	char* result = vsprintf(format,arg); 
	printstring(result);
	free(result);
	va_end(arg);
} 
 
char *convert(unsigned int num, int base) 
{ 
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50]; 
	char *ptr; 

	memset(&buffer,0,50);
	if(num==0){
		ptr = &buffer[0];
		buffer[0] = '0';
		return ptr;
	}
	
	ptr = &buffer[49]; 
	*--ptr = '\0'; 
	*--ptr = '\0'; 
	*--ptr = '\0'; 
	
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

/* PIXEL FORMAT
0b0000argb
a = alpha
r = red
g = green
b = blue
*/
void putpixel(int x,int y, unsigned char color) {
    unsigned where = x + (y * SCREEN_WIDTH);
	unsigned char* screen = (unsigned char*)0xA0000;
    screen[where] = color;
}

char getpixel(int x,int y){
	unsigned char* screen = (unsigned char*)0xA0000;
    	unsigned where = x + (y*SCREEN_WIDTH);
    	return screen[where];
}

void cls(){
	if(isGraphicsMode()){
		for(unsigned int x = 0 ; x < SCREEN_WIDTH ; x++){
			for(unsigned int y = 0 ; y < SCREEN_HEIGHT ; y++){
				putpixel(x,y,u_background);
			}
		}
	}else{
		for(cury = 0 ; cury < 25 ; cury++){
			for(curx = 0 ; curx < 80 ; curx++){
				vidpnt = (curx*2)+(160*cury);
				videomemory[vidpnt++] = ' ';
				videomemory[vidpnt++] = u_background;
			}
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

int init_graph_vga(unsigned int width, unsigned int height,int chain4) 
  // returns 1=ok, 0=fail
{
	const byte *w,*h;
	byte val;
	unsigned int a;
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	
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

unsigned long curget(){
	int tX = 0;
	int tY = 0;
	if(is_virtual_box_session_enabled()){
		long *t = vbox_get_mouse_info();
		if(t[0]){
			tX = t[0]/100;
		}
		if(t[1]){
			tY = t[1]/200;
		}
	}else{
		tX = ((int*)&curx)[0];
		tY = ((int*)&cury)[0];
	}
	unsigned long tow = 0;
	tow += (tX & 0xFFFF) + ((tY & 0xFFFF) * 0x10000); 
	return tow;
}

void curset(int x,int y){
	((int*)&curx)[0] = x;
	((int*)&cury)[0] = y;
}
