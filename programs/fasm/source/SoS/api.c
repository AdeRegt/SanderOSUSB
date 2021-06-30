#include <symbols.h>


extern int main(int a,char** b);

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
	char **c = (char *[]){"./fasm", filenamebuffer,fileoutbuffer,"-m","100"};
	main(5,c);
}