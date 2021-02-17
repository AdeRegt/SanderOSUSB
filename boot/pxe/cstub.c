
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

// unsigned char x = 0;
// unsigned char y = 0;

// void putch(char d){
//     if(d=='\n'){
//         x = 0;
//         y++;
//         return;
//     }
//     int ptr = ((y*80)+x)*2;
//     ((unsigned char*)0xb8000)[ptr] = d;
//     ((unsigned char*)0xb8000)[ptr+1] = 0x60;
//     x++;
//     if(x==80){
//         x = 0;
//         y++;
//     }
// }

// void printstring(char *message){
//     int i = 0;
//     while(1){
//         char deze = message[i++];
//         if(deze==0x00){
//             break;
//         }
//         putch(deze);
//     }
// }

// char *convert(unsigned int num, int base) 
// { 
// 	static char Representation[]= "0123456789ABCDEF";
// 	static char buffer[50]; 
// 	char *ptr; 
	
// 	ptr = &buffer[49]; 
// 	*ptr = '\0'; 
	
// 	do 
// 	{ 
// 		*--ptr = Representation[num%base]; 
// 		num /= base; 
// 	}while(num != 0); 
	
// 	return(ptr); 
// }

void relocate(unsigned long elfaddr,ELFSECTION section){
    // printstring("Loading ");
    // printstring(convert(section.sh_addr,16));
    // printstring(" with size ");
    // printstring(convert(section.sh_size,16));
    // printstring(" at offset ");
    // printstring(convert(section.sh_offset,16));
    // printstring(" \n");
    Elf32_Word e = 0;
    Elf32_Off oldaddr = 0;
    Elf32_Off newaddr = 0;
    unsigned char byteold = 0;
    onceagain:
    oldaddr = elfaddr + section.sh_offset + e;
    newaddr = section.sh_addr + e;
    byteold = ((unsigned char*)oldaddr)[0];
    ((unsigned char*)newaddr)[0] = byteold;
    if(e<section.sh_size){
        e++;
        goto onceagain;
    }
}

ELFHEADER *header;

void kernelpreloader(){
    // for(int i = 0 ; i < 256 ; i++){
    //     idt_set_gate(i, isr_common_stub, getCodeSegment(), 0x8E);
    // }
    // idt_load();
   // find ELF offset
   unsigned long elfaddr = 0x7C00;
   while(1){
       unsigned char A = ((unsigned char*)(elfaddr+0))[0];
       unsigned char B = ((unsigned char*)(elfaddr+1))[0];
       unsigned char C = ((unsigned char*)(elfaddr+2))[0];
       unsigned char D = ((unsigned char*)(elfaddr+3))[0];
       if(A==ELFMAG0&&B==ELFMAG1&&C==ELFMAG2&&D==ELFMAG3){
           goto t1;
       }
       elfaddr++;
   }
//    printstring("Found ELF header at :");
//    printstring(convert(elfaddr,16));
//    printstring("\n");
   // usually this value is 0x7F90
   t1:
   header = (ELFHEADER *)elfaddr;
   ELFSECTION * sections = (ELFSECTION *)((long)elfaddr + header->e_shoff);
   Elf32_Half headerc = header->e_shnum;
   for(Elf32_Half i = 0 ; i < headerc ; i++){
        ELFSECTION section = sections[i];
        if(section.sh_addr){
            if(section.sh_type==1){
                relocate(elfaddr,section);
            }
        }
   }
   void *(*foo)() = (void *) header->e_entry;
   foo();
   for(;;);
}