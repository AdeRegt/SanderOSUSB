#include <symbols.h>

void msg(const char *ms){
	typedef int func(const char*,...);
	func* f = (func*)F_PRINTF;// printf
	f(ms);
}

void putc(const char c){
	typedef int func(const char);
	func* f = (func*)F_PUTC;// putc
	f(c);
}

extern int main(int a,char** b);

char* filenamebuffer;
int long filesize;

void probeer(){
	msg("Hello world!");
	typedef char *funct();
	funct* ft = (funct*)F_BROWSE; //browse
	filenamebuffer = ft();
	char **c = (char *[]){"./fasm", filenamebuffer};
	//char** c = {"./fasm",ft};
	main(2,c);
}

int gettimeofday(unsigned long tv, unsigned long tz){
	return 0;
}

void *malloc(unsigned long size){
	return (void *)0x1000;
}

void free(void *ptr){
	
}

void exit(int status){
	if(status==0){
		int mode = 1;
		__asm__ __volatile__ (
		"int $0x80"
		: "+a" (mode)
	    	);
    	}
	for(;;);
}

const char *getenv(const char *name){
	return 0;
}

unsigned long fopen(const char* filename, const char *mode){	
	typedef char func(const char*);
	func* f = (func*)F_FEXISTS;// fexists
	//msg(filename);msg(mode);
	char o = f(filenamebuffer);
	return 0<o;
}

int fclose(unsigned long stream){
	return stream;
}

unsigned long fread(unsigned char *ptr, unsigned long size, unsigned long nmemb, unsigned long stream){
	typedef char func(char*,unsigned char*);
	func* f = (func*)F_FREAD; // fread
	
	f(filenamebuffer,ptr);
	
	filesize = 0;
	while(1){
		if(ptr[filesize++]==0x00){
			break;
		}
	}
	return 0;
}

unsigned long fwrite(const void *ptr, unsigned long size, unsigned long nmemb, unsigned long stream){
	typedef char func(char*,unsigned char*,unsigned long);
	func* f = (func*)F_FWRITE; // fread
	
	int t = 0;
	while(filenamebuffer[t]!=0x00){t++;}
	filenamebuffer[t-3] = 'b';
	filenamebuffer[t-2] = 'i';
	filenamebuffer[t-1] = 'n';
	
	f(filenamebuffer,(unsigned char*)ptr,(unsigned long)size);
	return nmemb;
}

int fseek(unsigned long stream, long int offset, int whence){
	return 0;
}

long int ftell(unsigned long stream){
	return filesize;
}

signed long write(int fd, const char *buf, unsigned long nbytes){
	putc(buf[0]);
	return 0;
}

unsigned long time(unsigned long t){
	return 0;
}
