#include "../kernel.h"
typedef unsigned long Elf32_Word;
typedef unsigned long Elf32_Addr;
typedef unsigned long Elf32_Off;
typedef unsigned long Elf32_Sword;
typedef unsigned short Elf32_Half;
#define ELFMAG0   0x7f
#define ELFMAG1   'E'
#define ELFMAG2   'L'
#define ELFMAG3   'F'
#define EI_NIDENT 16

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
} ELFHEADER;

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} ELFSECTION;

int iself(unsigned char* buffer){
	return buffer[0]==ELFMAG0&&buffer[1]==ELFMAG1&&buffer[2]==ELFMAG2&&buffer[3]==ELFMAG3;
}

unsigned long loadelf(void * buffer){
	ELFHEADER * header = (ELFHEADER *)buffer;
	if(header->e_type==2){
		ELFSECTION * sections = (ELFSECTION *)((long)buffer + header->e_shoff);
		for(unsigned int i = 0 ; i < header->e_shnum ; i++){
			ELFSECTION section = sections[i];
			if(section.sh_addr){
				if(section.sh_type==1){
					printf("ELF: loading segment at %x \n",section.sh_addr);
					for(unsigned int e = 0 ; e < section.sh_size ; e++){
						((char*)section.sh_addr)[e] = ((char*)buffer + section.sh_offset)[e];
					}
				}else{
					printf("ELF: no need to copy this segment \n");
				}
			}else{
				printf("ELF: skipping segment %x because it has no location\n",i);
			}
		}
		printf("ELF: ready at %x \n",header->e_entry);
		return header->e_entry;
	}else{
		printf("ELF: unknown elf type\n");
	}
	return 0;
}
