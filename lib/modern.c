#include "../include/string.h"
#define NULL 0

// defined in other lib
void *malloc(size_t size);
signed long write(int fd, const char *buf, unsigned long nbytes);
void free(void *ptr);
int getc(int stream);


char *itos(unsigned int num, int base) 
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

int strcmp(const char *str1, const char *str2){
    int p = 0;
    while(1){
        char A = str1[p];
        char B = str2[p];
        p++;
        if(A==0&&B==0){
            break;
        }else if(A==0||B==0){
            return -1;
        }else if(A!=B){
            return 1;
        }
    }
    return 0;
}

void *calloc(size_t nitems, size_t size){
    return (void *)malloc(nitems*size);
}

void *memcpy(void *dest, const void * src, size_t n){
    char *d = dest;
    const char *s = src;
    while (n--){
        *d++ = *s++;
    }
    return dest;
}

void *memset(void *str, int c, size_t n){
    unsigned char *ptr = str;
    while (n-- > 0){
        *ptr++ = c;
    }
    return str;
}

char *strncpy(char *dest, const char *src, size_t n){
    size_t size = strlen (src);
    if (size != n){
        memset (dest + size, '\0', n - size);
    }
    return memcpy (dest, src, size);
}

typedef int FILE;
typedef __builtin_va_list va_list;
#define va_start(a,b)  __builtin_va_start(a,b)
#define va_end(a)      __builtin_va_end(a)
#define va_arg(a,b)    __builtin_va_arg(a,b)
#define __va_copy(d,s) __builtin_va_copy((d),(s))

int vsnprintf(char *str, size_t size, char *format, va_list arg){
    if(strlen(format)==0){
		return NULL;
	}
	char* buffer = malloc(strlen(format));
	char* buffertmp;
	int buffersize = strlen(format) + 1;
	int oldsize;
	char *traverse; 
	unsigned int i; 
	signed int t;
	char *s;
	size_t travelpointer = 0;
	
	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		while( *traverse != '%' && *traverse != '\0' ) 
		{ 
			buffer[travelpointer++] = *traverse;
			traverse++; 
		} 
		if(*traverse =='\0'){
		    break; 
		}
		traverse++; 
		oldsize = buffersize;
		
		switch(*traverse) 
		{ 
			case 'c' : i = va_arg(arg,int);		//Fetch char argument
						// char is only 1 argument
						buffersize += 1;
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						buffer[travelpointer++] = i;
						break; 
						
			case 'd' : t = va_arg(arg,int); 		//Fetch Decimal/Integer argument
						char* chachacha;
						if(t<0) 
						{ 
							t = -t;
							chachacha = "-"; 
						} 
						chachacha = itos(t,10);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 'o': i = va_arg(arg,unsigned int); //Fetch Octal representation
						chachacha = itos(i,8);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 's': s = va_arg(arg,char *); 		//Fetch string
						chachacha = s; 
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
						chachacha = itos(i,16);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
				
		}	
	}
	buffer[travelpointer++] = 0x00;
    memcpy(str,buffer,(size_t)(travelpointer>size&&size!=0?size:travelpointer));
    return travelpointer;
}

int vfprintf(FILE *stream, char *format, va_list arg){
    if(strlen(format)==0){
		return NULL;
	}
	char* buffer = malloc(strlen(format));
	char* buffertmp;
	int buffersize = strlen(format) + 1;
	int oldsize;
	char *traverse; 
	unsigned int i; 
	signed int t;
	char *s;
	int travelpointer = 0;
	
	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		while( *traverse != '%' && *traverse != '\0' ) 
		{ 
			buffer[travelpointer++] = *traverse;
			traverse++; 
		} 
		if(*traverse =='\0'){
		    break; 
		}
		traverse++; 
		oldsize = buffersize;
		
		switch(*traverse) 
		{ 
			case 'c' : i = va_arg(arg,int);		//Fetch char argument
						// char is only 1 argument
						buffersize += 1;
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						buffer[travelpointer++] = i;
						break; 
						
			case 'd' : t = va_arg(arg,int); 		//Fetch Decimal/Integer argument
						char* chachacha;
						if(t<0) 
						{ 
							t = -t;
							chachacha = "-"; 
						} 
						chachacha = itos(t,10);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 'o': i = va_arg(arg,unsigned int); //Fetch Octal representation
						chachacha = itos(i,8);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 's': s = va_arg(arg,char *); 		//Fetch string
						chachacha = s; 
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
						
			case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
						chachacha = itos(i,16);
						buffersize += strlen(chachacha);
						buffertmp = buffer;
						buffer = malloc(buffersize);
						memcpy(buffertmp,buffer,oldsize);
						free(buffertmp);
						for(size_t x0 = 0 ; x0 < strlen(chachacha) ; x0++){buffer[travelpointer++] = chachacha[x0];}
						break; 
				
		}	
	}
	buffer[travelpointer++] = 0x00;
    return write((int)stream,buffer,strlen(buffer));
}

int printf(char *format, ...){
    va_list arg; 
	va_start(arg, format);
    int t = vfprintf((FILE*)1,format,arg);
	va_end(arg); 
    return t;
}

int sprintf(char *str, char *format, ...){
    va_list arg; 
	va_start(arg, format);
    int t = vsnprintf(str,0,format,arg);
	va_end(arg); 
    return t;
}

int fprintf(FILE *stream, char *format, ...){
	va_list arg; 
	va_start(arg, format);
    int t = vfprintf(stream,format,arg);
	va_end(arg); 
    return t;
}

char *readstring(char* tv,int size){
	for(int i = 0 ; i < size ; i++){
		char z = getc(1) & 0x000000FF;
		if(z=='\n'){
			break;
		}
		if(z=='\b'){
			i-= 2;
			continue;
		}
		tv[i] = z;
		write(1,(char*)&tv[i],1);
	}
	return tv;
}

int scanf ( const char * format, ... ){
	va_list arg; 
	va_start(arg, format);
	int tot = 0;
	for(size_t i = 0 ; i < strlen(format) ; i++){
		char t = format[i];
		if(t=='%'){
			i++;
			char u = format[i];
			if(u=='s'){}
		}else{
			tot++;
		}
	}
	va_end(arg);
	return tot; 
}

size_t strlen(const char *str){
    size_t result = 0;
	while(1){
		if(str[result]==0){
			break;
		}
		result++;
	}
	return result;
}

char* stpcpy(char *dest, const char *src){
    while((*dest++ = *src++) != '\0');
    return --dest;
}

int tolower(int c){
    return c+32;
}

long int strtol(const char *str, char **endptr, int base){
    long int result = 0;
	int pointer = strlen(str)-1;
	int min = -1;
	if(base==16){
		for(size_t i = 0 ; i < strlen(str) ; i++){
			if(str[i]=='x'){
				min = i;
			}
		}
	}
	int power = 1;
	for(int i = pointer ; i > min ; i--){
		char deze = str[i];
		if(base==10){
			if(deze>='0'&&deze<='9'){
				char t = deze-'0';
				result += (t*power);
			}
			power *= 10;
		}else if(base==16){
			int t = 0;
			if(deze>='0'&&deze<='9'){
				t = deze-'0';
			}else if(deze>='A'&&deze<='Z'){
				t = 10+(deze-'A');
			}else if(deze>='a'&&deze<='z'){
				t = 10+(deze-'a');
			}
			result += t*(16^(power-1));
			power++;
		}
	}
	*endptr = (char*) str;
	return result;
}

unsigned short switch_endian16(unsigned short nb) {
    return (nb>>8) | (nb<<8);
}

int isalpha(int c){
    return c > 64 && c < 123;
}

long long atoll(const char *nptr){
    return strtol (nptr, (char **) NULL, 10);
}

int isdigit(int c){
    return c > 47 && c < 58;
}

int system(const char *string){
    if(string==0){
        return -1; // show command processor is not avialable
    }else{
        return 0xCD; // function not found!
    }
}
