#include <symbols.h>


extern int main(int a,char** b);

void putc(const char c){
	typedef int func(const char);
	func* f = (func*)F_PUTC;
	f(c);
}

char* filenamebuffer;
char* fileoutbuffer;
int long filesize;

void probeer(){
	typedef char *funct();
	funct* ft = (funct*)F_BROWSE;
	filenamebuffer = ft();
	fileoutbuffer = (char*)0x9000;
	int t = 0;
	while(filenamebuffer[t]!=0x00){
		fileoutbuffer[t] = filenamebuffer[t];
		t++;
	}
	fileoutbuffer[t-4] = '.';
	fileoutbuffer[t-3] = 'b';
	fileoutbuffer[t-2] = 'i';
	fileoutbuffer[t-1] = 'n';
	char **c = (char *[]){"./fasm", filenamebuffer,fileoutbuffer,"-m","10"};
	main(5,c);
}

// int gettimeofday(unsigned long tv, unsigned long tz){
// 	return 0;
// }

// void exit(int status){
// 	if(status==0){
// 		int mode = 1;
// 		__asm__ __volatile__ (
// 		"int $0x80"
// 		: "+a" (mode)
// 	    	);
//     	}
// 	for(;;);
// }

// const char *getenv(const char *name){
// 	return "\0";
// }

// unsigned long fopen(const char* filename, const char *mode){	
// 	typedef char func(const char*);
// 	func* f = (func*)F_FEXISTS;
// 	char o = f(filenamebuffer);
// 	return 0<o;
// }

// int fclose(unsigned long stream){
// 	return stream;
// }

// unsigned long fread(unsigned char *ptr, unsigned long size, unsigned long nmemb, unsigned long stream){
// 	typedef char func(char*,unsigned char*);
// 	func* f = (func*)F_FREAD;
	
// 	f(filenamebuffer,ptr);
	
// 	filesize = 0;
// 	while(1){
// 		if(ptr[filesize++]==0x00){
// 			break;
// 		}
// 	}
// 	return 0;
// }

// unsigned long fwrite(const void *ptr, unsigned long size, unsigned long nmemb, unsigned long stream){
// 	typedef char func(char*,unsigned char*,unsigned long);
// 	func* f = (func*)F_FWRITE; // fread
	
// 	f(filenamebuffer,(unsigned char*)ptr,(unsigned long)size);
// 	return nmemb;
// }

// int fseek(unsigned long stream, long int offset, int whence){
// 	return 0;
// }

// long int ftell(unsigned long stream){
// 	return filesize;
// }

// signed long write(int fd, const char *buf, unsigned long nbytes){
// 	putc(buf[0]);
// 	return 0;
// }

// unsigned long time(unsigned long t){
// 	return 0;
// }