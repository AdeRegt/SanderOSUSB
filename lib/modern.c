#include "../include/string.h"
#define NULL 0

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

// defined in other lib
void *malloc(size_t size);

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

int ungetc(int chartoadd, FILE *stream){}

int getc(FILE *stream){}

char *realpath(const char * file_name,char * resolved_name){}

// defined in other lib
signed long write(int fd, const char *buf, unsigned long nbytes);

int printf(const char *format, ...){
    return write(1,format,strlen(format));
}

int sprintf(char *str, const char *format, ...){}

int fprintf(FILE *stream, const char *format, ...){}

int vfprintf(FILE *stream, const char *format, va_list arg){}

int vsnprintf(char *str, size_t size, const char *format, va_list ap){}

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

void *realloc(void *ptr, size_t size){}

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
        return result;
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
        return 0; // show command processor is not avialable
    }else{
        return 0xCD; // function not found!
    }
}