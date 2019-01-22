#include "../kernel.h"
unsigned char* videomemory = (unsigned char*)0xb8000;
//
// STRING
//
//


int vidpnt = 0;
int curx = 0;
int cury = 0;

void init_video(){
	// set cursor shape
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, (inportb(0x3D5) & 0xC0) | 0);
	outportb(0x3D4, 0x0B);
	outportb(0x3D5, (inportb(0x3D5) & 0xE0) | 15);
	// set cursor location
	vidpnt = 0;
	curx = 0;
	cury = 0;
}

void printstring(char* message){
	int a = 0;
	char b = 0;
	while((b=message[a++])!=0x00){
		putc(b);
	}
}

void putc(char a){
	if(a!='\n'){
		vidpnt = (curx*2)+(160*cury);
		videomemory[vidpnt++] = a;
		videomemory[vidpnt++] = 0x07;
		curx++;
	}
	if(curx==80||a=='\n'){
		curx = 0;
		cury++;
	}
	if(cury==25){
		cury = 24;	
		curx = 0;
		for(int i = 0 ; i < 24 ; i++){
			int v1dpnt = (160*(0+i));
			int v2dpnt = (160*(1+i));
			for(int z = 0 ; z < 160 ; z++){
				videomemory[v1dpnt+z] = videomemory[v2dpnt+z];
				videomemory[v2dpnt+z] = 0x00;
			}
		}
	}
}


void printf(char* format,...) 
{ 
	char *traverse; 
	unsigned int i; 
	signed int t;
	char *s; 
	
	//Module 1: Initializing Myprintf's arguments 
	va_list arg; 
	va_start(arg, format); 
	
	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		while( *traverse != '%' && *traverse != '\0' ) 
		{ 
			putc(*traverse);
			traverse++; 
		} 
		if(*traverse =='\0'){
		    break; 
		}
		traverse++; 
		
		//Module 2: Fetching and executing arguments
		switch(*traverse) 
		{ 
			case 'c' : i = va_arg(arg,int);		//Fetch char argument
						putc(i);
						break; 
						
			case 'd' : t = va_arg(arg,int); 		//Fetch Decimal/Integer argument
						if(t<0) 
						{ 
							t = -t;
							putc('-'); 
						} 
						printstring(convert(t,10));
						break; 
						
			case 'o': i = va_arg(arg,unsigned int); //Fetch Octal representation
						printstring(convert(i,8));
						break; 
						
			case 's': s = va_arg(arg,char *); 		//Fetch string
						printstring(s); 
						break; 
						
			case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
						printstring(convert(i,16));
						break; 
				
		}	
	} 
	
	//Module 3: Closing argument list to necessary clean-up
	va_end(arg); 
} 
 
char *convert(unsigned int num, int base) 
{ 
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

// FROM: https://wiki.osdev.org/Printing_To_Screen
char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void hexdump(unsigned long a){
	char msg[10];
	itoa(a,msg,16);
	printstring(msg);
}

