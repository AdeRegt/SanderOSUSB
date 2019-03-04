#include "../kernel.h"



Device devices['Z'-'A'];
int deviceint = 0;

Device *getNextFreeDevice(){
	int now = deviceint;
	deviceint++;
	return (Device *)&devices[now];
}

char whoopsie[100];
char* dir(char* path){
	for(int i = 0 ; i < 100 ; i++){
		whoopsie[i] = 0x00;
	}
	if(path[0]=='@'){
		int t = 0; 
		for(int i = 0 ; i < deviceint ; i++){
			if(t!=0){
				whoopsie[t++] = ';';
			}
			whoopsie[t++] = 'A'+i;
		}
	}else if(path[1]=='@'){
		int z = path[0] - 'A';
		if(devices[z].dir==0){
			
		}else{
			void* (*foo)(Device *,char*,char *) = (void*)devices[z].dir;
			foo((Device *)&devices[z],(unsigned char*)&path[2],whoopsie);
		}
	}
	return (char*) whoopsie;
}
