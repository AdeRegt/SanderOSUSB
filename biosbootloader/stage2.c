

unsigned char bootdevice = 0x00;


char getc(){
	asm("mov ax,0");
	asm("int 0x16");
}
 
char *convert(unsigned int num, int base){ 
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50]; 
	char *ptr; 
	
	ptr = &buffer[49]; 
	*ptr = '\0'; 
	
	do 
	{ 
		*--ptr = Representation[num%base]; 
		num /= base; 
	}while(num != 0); 
	
	return(ptr); 
}

void putc(char a){
	asm("mov ah,0x0e");
	asm("int 0x10");
}

void print_string(char* message){
	char deze = 0;
	int pointer = 0;
	while(1){
		deze = message[pointer];
		pointer++;
		if(deze==0x00){
			break;
		}
		putc(deze);
	}
}

void main(){
	print_string("\n\rHello world!\n\rBootdevice: ");
	print_string(convert(bootdevice,16));
	print_string("\n\r");
	unsigned char *buffer = (unsigned char*) 0x3000;
	print_string("Kernel starts with: ");
	putc(buffer[1]);
	putc(buffer[2]);
	putc(buffer[3]);
	for(;;);
}