#include "../kernel.h"

//
// MEMORY MANAGER
//
//

#define MEMORY_BLOCK_LIMIT 0x2000
typedef struct {
	unsigned long from;
	unsigned long to;
}MemoryBlock;

MemoryBlock memreg[MEMORY_BLOCK_LIMIT];

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

unsigned char getMemoryBlockUsedCount(){
	unsigned char count = 0;
	for(int i = 0 ; i < MEMORY_BLOCK_LIMIT ; i++){
		MemoryBlock *mb = (MemoryBlock*) (&memreg)+(sizeof(MemoryBlock)*i);
		if(!(mb->from==0&&mb->to==0)){
			count++;
		}
	}
	return count;
}

MemoryBlock *nextMemoryBlockAvailable(){
	for(int i = 0 ; i < MEMORY_BLOCK_LIMIT ; i++){
		MemoryBlock *mb = (MemoryBlock*) (&memreg)+(sizeof(MemoryBlock)*i);
		if(mb->from==0&&mb->to==0){
			return mb;
		}
	}
	return 0;
}

void *malloc_align(unsigned long size,unsigned long tag){
	MemoryBlock *memblck = nextMemoryBlockAvailable();
	if(((unsigned long)memblck)==0){
		printf("[PANIC] Out of memory (passed limit of %x ) !!!\n",MEMORY_BLOCK_LIMIT);
		for(;;);
	}

	unsigned long teller = 0;
	if(tag==0&&getMemoryBlockUsedCount()!=0){
		unsigned long tw = 0;
		for(int i = 0 ; i < MEMORY_BLOCK_LIMIT ; i++){
			MemoryBlock bl = memreg[i];
			if(bl.from==0&&bl.to==0){
				continue;
			}
			if(tw!=0&&tw!=bl.from){
				unsigned long df = 0;
				if(tw<bl.from){
					df = bl.from-tw;
				}else{
					df = tw-bl.from;
				}
				if(df>size){
					teller = tw;
					break;
				}
			}
			tw = bl.to;
		}
	}
	if(teller==0){
		// alligning stuff make things more difficult. 
		// so for now, we do the easy way
		for(int i = 0 ; i < MEMORY_BLOCK_LIMIT ; i++){
			MemoryBlock bl = memreg[i];
			if(bl.from==0&&bl.to==0){
				continue;
			}
			if(teller<bl.to){
				teller = bl.to;
			}
		}
	}

	if(teller==0){
		teller = 0x30000;
	}

	while(1){
		unsigned long Y = teller&tag;
		if(Y==0){
			break;
		}
		teller++;
	}

	memblck->from = teller;
	memblck->to = teller + size;

	for(unsigned long i = memblck->from ; i < memblck->to ; i++){
		((unsigned char*)(memblck->from+i))[0] = 0;
	}

	//printf("[MEM] Allocated memory: FROM=%x TO=%x SIZE=%x\n",memblck->from,memblck->to,size);
	return (void *)memblck->from;
}

void free(void *loc){
	for(int i = 0 ; i < MEMORY_BLOCK_LIMIT ; i++){
		MemoryBlock *mb = (MemoryBlock*) (&memreg)+(sizeof(MemoryBlock)*i);
		if(mb->from==(unsigned long)loc){
			mb->from = 0;
			mb->to = 0;
		}
	}
}

void *malloc(unsigned long size){
	return malloc_align(size,0);
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

