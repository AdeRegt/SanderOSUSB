#include "../../kernel.h"

#define PAGE_DIRECTORY_SIZE 1024

extern void loadPageDirectory(unsigned long*);
extern void enablePaging();

unsigned long page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
unsigned long first_page_table[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));

void paging_clear_directory_pages(){
    for(int i = 0 ; i < PAGE_DIRECTORY_SIZE ; i++){
        page_directory[i] = 0x00000002;
    }
}

void init_paging(){
    printf("[PAGING] Setting up paging...\n");
    printf("[PAGING] Directory table at %x with size %x \n",&page_directory,PAGE_DIRECTORY_SIZE);
    paging_clear_directory_pages();

    //
    // minimal setup, create one pagedir with linear address 1:1
    for(int i = 0 ; i < PAGE_DIRECTORY_SIZE ; i++){
        first_page_table[i] = (i * 0x1000) | 3;
    }
    page_directory[0] = ((unsigned int)first_page_table) | 3;

    printf("[PAGING] Tell CPU where page tables are\n");
    loadPageDirectory(page_directory);

    printf("[PAGING] Tell CPU to use paging\n");
    enablePaging();

    printf("[PAGING] Paging is enabled\n");
}