#include <symbols.h>

#ifndef D_MSG
	#define D_MSG
	void msg(const char *ms);
#endif

#ifndef lock_filepointer
	#define lock_filepointer
	typedef int FILE;
	#define EOF 0
	FILE *stdout;
	FILE *stderr;
#endif

#ifndef D_fopen
	#define D_fopen 
	FILE *fopen(const char *filename, const char *mode);
#endif

#ifndef D_fclose
	#define D_fclose 
	int fclose(FILE *stream);
#endif

#ifndef D_FPRINTF
	#define D_FPRINTF
	int fprintf(FILE *stream, const char *format, ...);
#endif

#ifndef D_SPRINTF
	#define D_SPRINTF
	int sprintf(char *str, const char *format, ...);
#endif

#ifndef D_PRINTF
	#define D_PRINTF
	int printf(const char *format, ...);
#endif

#ifndef D_UNGETC
	#define D_UNGETC
	int ungetc(int chartoadd, FILE *stream);
#endif

#ifndef D_GETC
	#define D_GETC
	int getc(FILE *stream);
#endif

#ifndef _STDARG_H_
#define _STDARG_H_

typedef __builtin_va_list va_list;

#define va_start(a,b)  __builtin_va_start(a,b)
#define va_end(a)      __builtin_va_end(a)
#define va_arg(a,b)    __builtin_va_arg(a,b)
#define __va_copy(d,s) __builtin_va_copy((d),(s))

#ifndef D_NULL
	#define D_NULL
	#define NULL (void*)0
#endif 

#endif // _STDARG_H_

#ifndef D_VFPRINTF
	#define D_VFPRINTF

	int vfprintf(FILE *stream, const char *format, va_list arg);
#endif 

#ifndef D_size_t
    #define D_size_t
    typedef unsigned int size_t;
#endif

#ifndef D_VSNPRINTF
#define D_VSNPRINTF
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

#ifndef D_FREAD
	#define D_FREAD
	size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
#endif 

signed long write(int fd, const char *buf, unsigned long nbytes);
int scanf ( const char * format, ... );
char *readstring(char* tv,int size);