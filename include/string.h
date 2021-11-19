#ifndef D_strcmp
    #define D_strcmp
    int strcmp(const char *str1, const char *str2);
#endif

#ifndef D_size_t
    #define D_size_t
    typedef unsigned int size_t;
#endif

#ifndef D_strlen
    #define D_strlen
    size_t strlen(const char *str);
#endif

#ifndef D_memcpy
    #define D_memcpy
    void *memcpy(void *dest, const void * src, size_t n);
#endif

#ifndef D_MEMSET
    #define D_MEMSET
    void *memset(void *str, int c, size_t n);
#endif

#ifndef D_STPCPY
    #define D_STPCPY
    char* stpcpy(char *a, const char *b);
#endif

#ifndef D_STRNCPY
    #define D_STRNCPY
    char *strncpy(char *dest, const char *src, size_t n);
#endif

#ifndef D_STRLEN
    #define D_STRLEN
    size_t strlen(const char *str);
#endif 