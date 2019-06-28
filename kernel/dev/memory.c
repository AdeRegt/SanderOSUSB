#include "../kernel.h"

//
// MEMORY MANAGER
//
//

unsigned char *memman = (unsigned char*) 0x30000;
int mempoint = 0;

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
		((unsigned char*)str)[t] = c;
	}
	return str;
}

int memcmp( char *str1,  char *str2, int n){
	for(int i = 0 ; i < n ; i++){
		if(str1[i]!=str2[i]){
			return 1;
		}
	}
	return 0;
}

