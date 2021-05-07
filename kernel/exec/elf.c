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

enum ELFSECTIONTYPES {
	ELFSECTIONTYPES_NULL		= 0,   // Null section
	ELFSECTIONTYPES_PROGBITS	= 1,   // Program information
	ELFSECTIONTYPES_SYMTAB		= 2,   // Symbol table
	ELFSECTIONTYPES_STRTAB		= 3,   // String table
	ELFSECTIONTYPES_RELA		= 4,   // Relocation (w/ addend)
	ELFSECTIONTYPES_NOBITS		= 8,   // Not present in file
	ELFSECTIONTYPES_REL			= 9,   // Relocation (no addend)
};

enum ELFSECTIONATTRIBUTES {
	ELFSECTIONATTRIBUTES_WRITE	= 0x01, // Writable section
	ELFSECTIONATTRIBUTES_ALLOC	= 0x02  // Exists in memory
};

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

enum ELFRELOCATIONTYPE{ // 0xA 0x9 0x4 nog over
	ELFRELOCATIONTYPE_386_NONE		= 0, // No relocation
	ELFRELOCATIONTYPE_386_32		= 1, // Symbol + Offset
	ELFRELOCATIONTYPE_386_PC32		= 2  // Symbol + Offset - Section Offset
};

typedef struct {
	Elf32_Addr r_offset;
	Elf32_Word r_info;
}ELFRELOCATION;

typedef struct {
	Elf32_Word		st_name;
	Elf32_Addr		st_value;
	Elf32_Word		st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half		st_shndx;
} ELFSYMBOL;

int iself(unsigned char* buffer){
	return buffer[0]==ELFMAG0&&buffer[1]==ELFMAG1&&buffer[2]==ELFMAG2&&buffer[3]==ELFMAG3;
}

void elf_error_message(){
	printf("Invalid call\n");
	for(;;);
}

int elf_get_symval(ELFHEADER* header, int table,unsigned int info){
	if(table==0||info==0){
		printf("ELF: get_symfal is 0 \n");
		return 0;
	}
	ELFSECTION symtab = ((ELFSECTION*)((int)header + header->e_shoff))[table];
	ELFSYMBOL symbol = ((ELFSYMBOL*)((int)header+symtab.sh_offset))[info];
	if(symbol.st_shndx==0){
		// external symbol
		ELFSECTION strtab = ((ELFSECTION*)((int)header + header->e_shoff))[symtab.sh_link];
		char *name = (char *)header + strtab.sh_offset + symbol.st_name;
		if(name){
			if(memcmp(name,"printf",strlen("printf"))==0){
				return (unsigned long)printf;
			}else{
				return (unsigned long)elf_error_message;
			}
		}
		return -1;
	}else if(symbol.st_shndx==10234){
		return symbol.st_value;
	}else{
		ELFSECTION target = ((ELFSECTION*)((int)header + header->e_shoff))[symbol.st_shndx];
		return (int) header + symbol.st_value + target.sh_offset;
	}
}

unsigned long loadelf(void * buffer){
	ELFHEADER * header = (ELFHEADER *)buffer;
	if(header->e_ident[4]!=1){
		printf("ELF: Invalid type. Target is 32 bit\n");for(;;);
	}
	if(header->e_type==1){
		int ok = 0;
		ELFSECTION * sections = (ELFSECTION *)((long)buffer + header->e_shoff);
		for(unsigned int i = 0 ; i < header->e_shnum ; i++){
			ELFSECTION section = sections[i];
			if(section.sh_type==ELFSECTIONTYPES_NOBITS){
				if(section.sh_size && section.sh_flags & ELFSECTIONATTRIBUTES_ALLOC){
					void *meminfo = malloc(section.sh_size);
					section.sh_offset = (int) meminfo - (int) header;
					printf("ELF: allocated a nobits section of the size %x \n",section.sh_offset);
				}
			}else if(section.sh_type==ELFSECTIONTYPES_REL){
				for(unsigned int j = 0 ; j < (section.sh_size / section.sh_entsize) ; j++){
					ELFRELOCATION* relocation = (ELFRELOCATION*)(((int)header) + section.sh_offset + (sizeof(ELFRELOCATION)*j));
					ELFSECTION target = ((ELFSECTION*)((int)header + header->e_shoff))[section.sh_info];
					int *ref = (int *)(((int) header + target.sh_offset) + relocation->r_offset);
					int symval = 0;
					if((relocation->r_info>>8) != 0){
						symval = elf_get_symval(header,section.sh_link,relocation->r_info >> 8);
					}
					if(symval==-1){
						ok = 0;
					}
					if(((unsigned char)relocation->r_info)==ELFRELOCATIONTYPE_386_NONE){
						// no need to do something
					}else if(((unsigned char)relocation->r_info)==ELFRELOCATIONTYPE_386_32){
						*ref = ((symval) + (*ref));
					}else if(((unsigned char)relocation->r_info)==ELFRELOCATIONTYPE_386_PC32){
						*ref = ((symval) + (*ref) - ((int)ref));
					}else{
						printf("ELF: Unknown relocation type: %x \n",((unsigned char)relocation->r_info));
						continue;
					}
					printf("ELF: Relocating symbol\n");
				}
			}else if(section.sh_type==ELFSECTIONTYPES_PROGBITS){
				if(ok==0){
					ok = section.sh_offset;
				}
			}
		}
		printf("ELF: finished loading allocated elf\n");
		if(ok){
			return (int)header + ok;
		}else{
			for(;;);
		}
	}else if(header->e_type==2){
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
		printf("ELF: unknown elf type: %x \n",header->e_type);
	}
	return 0;
}
