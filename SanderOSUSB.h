void exit (void){
	int mode = 1;
	__asm__ __volatile__ (
        "int $0x80"
        : "+a" (mode)
    	);
}

void print(const char* message,int to){
	int mode = 4;
	int len = 0;
	while(message[len++]!=0x00);
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
