#include <symbols.h>

#ifndef D_EXIT
	#define D_EXIT
	void exit(int status);
#endif

#ifndef D_FREE
	#define D_FREE
	void free(void *ptr);
#endif

#ifndef D_size_t
    #define D_size_t
    typedef unsigned int size_t;
#endif

#ifndef D_MALLOC
	#define D_MALLOC
	void *malloc(size_t size);
#endif

#ifndef D_CALLOC
	#define D_CALLOC
	void *calloc(size_t nitems, size_t size);
#endif

#ifndef D_REALPATH
	#define D_REALPATH
	char *realpath(const char * file_name,char * resolved_name);
#endif 

#ifndef D_REALLOC
	#define D_REALLOC
	void *realloc(void *ptr, size_t size);
#endif

#ifndef D_STRTOL
	#define D_STRTOL
	long int strtol(const char *str, char **endptr, int base);
	int atoi(const char *nptr);
	long atol(const char *nptr);
	long long atoll(const char *nptr);
	long long atoq(const char *nptr);
#endif

#ifndef D_SYSTEM
	#define D_SYSTEM
	int system(const char *string);
#endif