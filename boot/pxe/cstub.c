
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

unsigned char x = 0;
unsigned char y = 0;

void putch(char d){
    int ptr = ((y*25)+x)*2;
    ((unsigned char*)0xb8000)[ptr] = d;
    ((unsigned char*)0xb8000)[ptr+1] = 0x60;
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
   ELFHEADER * header = (ELFHEADER *)elfaddr;
   if(header->e_type==1){
       
   }
   for(;;);
}