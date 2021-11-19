#define STDIO 1

#ifndef NULL
    #define NULL 0
#endif 

#ifndef D_EXIT
	#define D_EXIT
    void exit (void){
        int mode = 1;
        __asm__ __volatile__ (
            "int $0x80"
            : "+a" (mode)
            );
    }
#endif

void print(const char* message,int to){
	int mode = 4;
	int len = 0;
	while(message[len++]!=NULL);
	__asm__ __volatile__ (
        "int $0x80"
        : "+a" (mode)
        , "+b" (to)
        , "+c" (message)
        , "+d" (len)
    	);
}

void read(int to,char *message,int len){
	int mode = 3;
	__asm__ __volatile__ (
        "int $0x80"
        : "+a" (mode)
        , "+b" (to)
        , "+c" (message)
        , "+d" (len)
    	);
}

int open(const char *pathname, int flags, int mode){
	int model = 5;
	int res = 0;
	__asm__ __volatile__ (
        "int $0x80"
        : "+a" (model)
        , "+b" (pathname)
        , "+c" (flags)
        , "+d" (mode)
        :
        "r" (res)
    	);
    	return res;
}

void printf(const char* msg,...){
    void* (*foo)(const char*,...) = (void*) 0x100012;
	foo(msg);
}
