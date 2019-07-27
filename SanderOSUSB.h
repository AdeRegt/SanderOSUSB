void exit (void){
	int mode = 1;
	__asm__ __volatile__ (
        "int $0x80"
        : "+a" (mode)
    	);
}

void print (const char* message,int to){
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
