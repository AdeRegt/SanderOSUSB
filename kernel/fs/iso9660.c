#include "../kernel.h"


unsigned long charstoint(unsigned char a,unsigned char b,unsigned char c,unsigned char d){
        unsigned long final = 0;
        final |= ( a << 24 );
        final |= ( b << 16 );
        final |= ( c <<  8 );
        final |= ( d       );
        return final;
}

	unsigned char pathpart[30];
	
unsigned long iso_9660_target(Device *device,char* path,char *buffer){
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	//
	// eerst opzoeken van de primaire block
	//
	
	int primairesector = 0;
	for(int i = 0 ; i < 10 ; i++){
		readraw(device,0x10+i,1,(unsigned short *)buffer);
		if(buffer[0]==0x01&&buffer[1]=='C'&&buffer[2]=='D'&&buffer[3]=='0'&&buffer[4]=='0'&&buffer[5]=='1'){
			primairesector = 0x10+i;
			break;
		}
	}
	
	if(primairesector==0){
		printf("ISO: primairy sector not found!\n");for(;;);
	}
	
	unsigned long dt = charstoint(buffer[148],buffer[149],buffer[150],buffer[151]);
	readraw(device,dt,1,(unsigned short *)buffer);
	
	unsigned long res = charstoint(buffer[2],buffer[3],buffer[4],buffer[5]);
	if(path[0]==0){
		return res;
	}
	int pathpointer = 0;
	char tot = 1;
	int tcnt = 0;
	
	nextpathelem:
	tcnt = 0;
	for(int i = 0 ; i < 30 ; i++){
		char deze = path[pathpointer++];
		if(deze==0x00||deze=='/'||deze=='#'){
			pathpart[i]=0;
			tcnt = i;
			break;
		}
		pathpart[i]=deze;
	}
	
	int found = 0;
	char prx = 0;
	int i = 0;
	for(int y = 0 ; y < 10 ; y++){
		char ttutA = buffer[i+0];
		char ttutB = buffer[i+1];
		char ttutC = buffer[i+7];
		if(tot==ttutC){
//			putc('\"');
//			for(int z = 0 ; z < ttutA ; z++){
//				putc(buffer[i+8+z]);
//			}
//			putc('\"');
			if(tcnt==ttutA){
				found = 1;
				for(int z = 0 ; z < ttutA ; z++){
					char A = buffer[i+8+z];
					char B = pathpart[z];
					if(A!=B){
						found = 0;
					}
				}
				if(found==1){
					tot = y+1;
					res = charstoint(buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5]);
				}
			}
		}
		int z = ttutA+ttutB+8;
		if(z%2!=0){
			z++;
		}
		i += z;
	}
	if(found){
		char deze = path[pathpointer-1];
		if(deze==0x00||deze=='#'){
			goto foundyay;
		}else{
			goto nextpathelem;
		}
	}
	res = 0;
	return res;
	
	foundyay:
	return res;
}

void iso_9660_dir(Device *device,char* path,char *buffer){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	int target = iso_9660_target(device,path,buffer);
	if(target!=0){
		readraw(device,target,1,(unsigned short *)buffer);
		int tor = 0;
		int gz = 0;
		for(int i = 0 ; i < 1000 ; i++){
			int t = 2;
			if(buffer[i]==';'&&buffer[i+1]=='1'){
				int fnd = 0;
				for(int z = 1 ; z < 30 ; z++){
					if(buffer[i-z]==t){
						fnd = z;
						break;
					}
					t++;
				}
				if(fnd){
					//t -= 2;
					if(gz){
						buffer[tor++] = ';';
					}
					for(int z = 2 ; z < t ; z++){
						buffer[tor++] = buffer[(i-t)+z];
					}
					gz++;
				}
			}
		}
		buffer[tor++] = 0x00;
	}else{
		buffer[0]=0x00;
	}
}
