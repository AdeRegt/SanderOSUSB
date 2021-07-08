#include "../../kernel.h"

#define PAGE_DIRECTORY_SIZE 1024

extern void loadPageDirectory(unsigned long*);
extern void enablePaging();

unsigned long page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
//unsigned long first_page_table[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));

void paging_clear_directory_pages(){
    for(int i = 0 ; i < PAGE_DIRECTORY_SIZE ; i++){
        page_directory[i] = 0x00000002;
    }
}

unsigned int pagecount = 0; 

void set_paging_frame(unsigned long addr){
    unsigned long addr2 = addr;
    addr2 -= (addr2 & 0xFFF);
    debugf("[PAGING] Create page for %x \n",addr2);
    long *first_page_table = malloc_align(PAGE_DIRECTORY_SIZE*sizeof(unsigned long),4096);
    for(int i = 0 ; i < PAGE_DIRECTORY_SIZE ; i++){
        first_page_table[i] = (addr2+(i * 0x1000))  | 0x3;
    }
    page_directory[pagecount++] = ((unsigned int)first_page_table) | 0x3;
}

void init_paging(){
    debugf("[PAGING] Setting up paging...\n");
    debugf("[PAGING] Directory table at %x with size %x \n",&page_directory,PAGE_DIRECTORY_SIZE);
    paging_clear_directory_pages();

    //
    // minimal setup, create one pagedir with linear address 1:1
    set_paging_frame(0);

    debugf("[PAGING] Tell CPU where page tables are\n");
    loadPageDirectory(page_directory);

    debugf("[PAGING] Tell CPU to use paging\n");
    #ifdef ENABLE_PAGING
    enablePaging();
    #endif

    debugf("[PAGING] Paging is enabled\n");
}