
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

unsigned char x = 0;
unsigned char y = 0;

void putch(char d){
    if(d=='\n'){
        x = 0;
        y++;
        return;
    }
    int ptr = ((y*80)+x)*2;
    ((unsigned char*)0xb8000)[ptr] = d;
    ((unsigned char*)0xb8000)[ptr+1] = 0x60;
    x++;
    if(x==80){
        x = 0;
        y++;
    }
}

void printstring(char *message){
    int i = 0;
    while(1){
        char deze = message[i++];
        if(deze==0x00){
            break;
        }
        putch(deze);
    }
}

void kernelpreloader(){
   // find ELF offset
   unsigned long elfaddr = 0x7C00;
   while(1){
       unsigned char A = ((unsigned char*)(elfaddr+0))[0];
       unsigned char B = ((unsigned char*)(elfaddr+1))[0];
       unsigned char C = ((unsigned char*)(elfaddr+2))[0];
       unsigned char D = ((unsigned char*)(elfaddr+3))[0];
       if(A==ELFMAG0&&B==ELFMAG1&&C==ELFMAG2&&D==ELFMAG3){
           putch('X');
           break;
       }
       elfaddr++;
   }
   printstring("Found ELF header\n");
   ELFHEADER * header = (ELFHEADER *)elfaddr;
   ELFSECTION * sections = (ELFSECTION *)((long)elfaddr + header->e_shoff);
   for(unsigned int i = 0 ; i < header->e_shnum ; i++){
        ELFSECTION section = sections[i];
        if(section.sh_addr){
            if(section.sh_type==1){
                for(unsigned int e = 0 ; e < section.sh_size ; e++){
                    ((char*)section.sh_addr)[e] = ((char*)elfaddr + section.sh_offset)[e];
                }
            }
        }
   }
   void *(*foo)() = (void *)header->e_entry;
   foo();
   for(;;);
}