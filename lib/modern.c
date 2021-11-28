#include "../include/string.h"

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
    return (void *)malloc(size);
}

void *memcpy(void *dest, const void * src, size_t n){
    
}

void *memset(void *str, int c, size_t n){

}

char *strncpy(char *dest, const char *src, size_t n){

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

int printf(const char *format, ...){}

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

char* stpcpy(char *a, const char *b){}

int tolower(int c){}

long int strtol(const char *str, char **endptr, int base){}

int isalpha(int c){}

long long atoll(const char *nptr){}

int isdigit(int c){}

int system(const char *string){}