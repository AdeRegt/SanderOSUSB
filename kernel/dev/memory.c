#include "../kernel.h"

//
// MEMORY MANAGER
//
//

unsigned char *memman = (unsigned char*) 0x1000;

void *malloc(unsigned long size){
	unsigned long currentloc = (unsigned long)&memman;
	for(unsigned long i = 0 ; i < size ; i++){
		memman[i] = 0x00;
	}
	memman += size;
	return (void *)currentloc;
}

