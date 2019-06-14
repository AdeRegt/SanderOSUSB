#include "../kernel.h"



Device devices['Z'-'A'];
int deviceint = 0;

int getDeviceCount(){
	return deviceint;
}

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
			void* (*foo)(Device *,unsigned char*,char *) = (void*)devices[z].dir;
			foo((Device *)&devices[z],(unsigned char*)&path[2],whoopsie);
		}
	}
	return (char*) whoopsie;
}

char fexists(unsigned char* path){
	for(int i = 0 ; i < 100 ; i++){
		whoopsie[i] = 0x00;
	}
	if(path[1]=='@'){
		int z = path[0] - 'A';
		if(devices[z].existsFile==0){
			return 0;
		}else{
			char (*foo)(Device *,unsigned char*) = (void *)devices[z].existsFile;
			char result = (char)foo((Device *)&devices[z],(unsigned char*)&path[2]);
			return result;
		}
	}else if(path[0]==0x00){
		return 0;
	}else{
		//return 0;
			if(getDeviceCount()>0){
				if(devices[0].existsFile==0){
					return 0;
				}
				char (*foo)(Device *,unsigned char*) = (void *)devices[0].existsFile;
				return (char) foo((Device *)&devices[0],path);
			}else{
				return 0;
			}
	}
}

void fread(char* path,unsigned char* buffer){
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
		if(devices[z].readFile==0){
			
		}else{
			void* (*foo)(Device *,unsigned char*,unsigned char *) = (void*)devices[z].readFile;
			foo((Device *)&devices[z],(unsigned char*)&path[2],buffer);
		}
	}
}
