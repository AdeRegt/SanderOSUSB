#include "../kernel.h"

//
// MEMORY MANAGER
//
//

#define MEMORY_BLOCK_LIMIT 0x2000
typedef struct {
	void *from;
	void *to;
	unsigned char used;
}MemoryBlock;

MemoryBlock memreg[MEMORY_BLOCK_LIMIT];

unsigned char *memman = (unsigned char*) 0x30000;
int mempoint = 0;

extern int curx;
extern int cury;
extern void *endKernelMemory;

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

void memswapentries(int pointA,int pointB){
	void *buffrom = memreg[pointA].from;
	void *bufto = memreg[pointA].to;
	unsigned char bufuse = memreg[pointA].used;
	memreg[pointA].from = memreg[pointB].from;
	memreg[pointA].to = memreg[pointB].to;
	memreg[pointA].used = memreg[pointB].used;
	memreg[pointB].from = buffrom;
	memreg[pointB].to = bufto;
	memreg[pointB].used = bufuse;
}

/**
 * Structures everything from high to low
 */
void memcleanup(){
	int foundsomething = 1;
	while(foundsomething){
		foundsomething = 0;
		for(int i1 = 1 ; i1 < MEMORY_BLOCK_LIMIT ; i1++){
			int i0 = i1-1;
			MemoryBlock blockA = memreg[i0];
			MemoryBlock blockB = memreg[i1];
			if(blockA.from<blockB.from){
				memswapentries(i0,i1);
				foundsomething = 1;
			}
			if(blockA.from==0&&blockB.from==0){
				break;
			}
		}
	}
}

void *malloc_align(unsigned long size,unsigned long tag){
	// from high to low....
	memcleanup();
	// check for some space in already existing terretories...
	int found_something = -1;
	for(int i1 = 0 ; i1 < MEMORY_BLOCK_LIMIT ; i1++){
		if(memreg[i1].used==0&&memreg[i1].from!=0&&memreg[i1].to!=0){
			unsigned int sizeofblock = memreg[i1].to-memreg[i1].from;
			if(sizeofblock==size&&(((unsigned long)memreg[i1].from)&tag)==0){
				// perfect fit... yay!
				found_something = i1;
			}
		}
	}
	// do we have to assign a new block?
	if(found_something==-1){
		if(memreg[MEMORY_BLOCK_LIMIT-1].used==0&&memreg[MEMORY_BLOCK_LIMIT-1].from==0&&memreg[MEMORY_BLOCK_LIMIT-1].to==0){
			// get highest address
			int highestlocation = (int)memreg[0].to;
			// if not pressent, take default address
			if(highestlocation==0){
				highestlocation = (int)&endKernelMemory;
			}
			// match alignment
			while(1){
				if((highestlocation&tag)==0){
					break;
				}
				highestlocation++;
			}
			// find latest spot
			for(int i1 = 0 ; i1 < MEMORY_BLOCK_LIMIT ; i1++){
				if(memreg[i1].used==0&&memreg[i1].from==0&&memreg[i1].to==0){
					memreg[i1].from = (void*)highestlocation;
					memreg[i1].to = (void*)(highestlocation+size);
					found_something = i1;
					break;
				}
			}
		}
	}
	// if still nothing is found...
	if(found_something==-1){
		printf("[MEM] Out of memory\n");
		for(;;);
	}
	// mark as used
	memreg[found_something].used = 1;
	// cleanup
	for(unsigned int i = 0 ; i < size ; i++){
		((unsigned char*)(memreg[found_something].from))[i] = 0;
	}
	// release
	return memreg[found_something].from;
}

void free(void *loc){
	for(int i = 0 ; i < MEMORY_BLOCK_LIMIT ; i++){
		MemoryBlock *mb = (MemoryBlock*) (&memreg)+(sizeof(MemoryBlock)*i);
		if(mb->from==loc){
			mb->used = 0;
			memset(mb->from,0,mb->to-mb->from);
		}
	}
	memcleanup();
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

ArrayListElement* createArrayList(void *initialitem){
	ArrayListElement *element = (ArrayListElement*)malloc(sizeof(ArrayListElement));
	element->locationToItem = initialitem;
	return element; 
}

void appendElementToExistingArrayList(ArrayListElement* original,ArrayListElement* extraitem){
	extraitem->previous = original;
	original->next = extraitem;
}

ArrayListElement* appendToExistingArrayList(ArrayListElement* list,void *item){
	ArrayListElement *newelement = createArrayList(item);
	appendElementToExistingArrayList(list,newelement);
	return newelement;
}

void foreach(ArrayListElement* list,unsigned long functionpointer){
	ArrayListElement *next = list;
	void* (*each)(void*) = (void*)functionpointer;
	while(1){
		each(next->locationToItem);
		if(next->next==0){
			break;
		}
		next = (ArrayListElement *)next->next;
	}
}