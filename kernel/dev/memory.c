#include "../kernel.h"

//
// MEMORY MANAGER
//
//

unsigned char *memman = (unsigned char*) 0x30000;
int mempoint = 0;

extern int curx;
extern int cury;

void memdump(unsigned long location){
	unsigned char* buffer = (unsigned char*) location;
	unsigned long tar = 0;
	while(1){
		cls();
		curx = 0;
		cury = 0;
		for(int i = 0 ; i < 24 ; i++){
			printf("                                                           \n");
		}
		curx = 0;
		cury = 0;
		printf(" ADRESS   |   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
		printf("----------+--------------------------------------------------\n");
		for(int y = 0 ; y < 20 ; y++){
			curx = 1;
			cury = y+2;
			printf("%x",buffer+tar+(y*0x10));
			curx = 10;
			printf("| ");
			for(int x = 0 ; x < 16 ; x++){
				unsigned char deze = buffer[tar+(y*0x10)+x];
				curx = 14+(x*3);
				if(!(deze>0xF)){
					printf("0");
				}
				printf("%x",deze);
			}
		}
		if(0){
			resetTicks();
			while(getTicks()<50);
			tar += 0x100;
		}else{
			return;
		}
	}
}

void *malloc_align(unsigned long size,unsigned long tag){
	while(1){
		unsigned long tx = (unsigned long)&memman[mempoint];
		if((tx&tag)==0){
			break;
		}
		mempoint++;
	}
	unsigned long currentloc = (unsigned long)&memman[mempoint];
	for(unsigned long i = 0 ; i < size ; i++){
		memman[mempoint+i] = 0x00;
	}
	mempoint += size;
	return (void *)currentloc;
}

void *malloc(unsigned long size){
	unsigned long currentloc = (unsigned long)&memman[mempoint];
	for(unsigned long i = 0 ; i < size ; i++){
		memman[mempoint+i] = 0x00;
	}
	mempoint += size;
	return (void *)currentloc;
}

 
void *memset(void *str, int c, int n){
	for(int t = 0 ; t < n ; t++){
		// why do we ask for an int and
		//  then use only a byte from it???
		((unsigned char*)str)[t] = c;
	}
	return str;
}

int memcmp(char *str1,  char *str2, int n){
	for(int i = 0 ; i < n ; i++){
		if(str1[i] != str2[i]){
			return 1;
		}
	}
	return 0;
}

void memcpy( char *from,  char *to, int n){
	for(int i = 0 ; i < n ; i++){
		to[i] = from[i];
	}
}

int strlen(char *str){
	int count = 0;
	while(str[count] != '\0') {
		count++;
	}
	return count;
}

