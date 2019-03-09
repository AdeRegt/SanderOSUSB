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
	unsigned char selfloor = 1;
	volatile unsigned char* isobuffer = (volatile unsigned char*)0x1000;
	unsigned long isonameloc = 0;
	
unsigned long iso_9660_target(Device *device,char* path){
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	//
	// eerst opzoeken van de primaire block
	//
	
	int primairesector = 0;
	for(int i = 0 ; i < 10 ; i++){
		readraw(device,0x10+i,1,(unsigned short *)isobuffer);
		if(isobuffer[0]==0x01&&isobuffer[1]=='C'&&isobuffer[2]=='D'&&isobuffer[3]=='0'&&isobuffer[4]=='0'&&isobuffer[5]=='1'){
			primairesector = 0x10+i;
			break;
		}
	}
	
	if(primairesector==0){
		printf("ISO: primairy sector not found!\n");for(;;);
	}
	
	unsigned long dt = charstoint(isobuffer[148],isobuffer[149],isobuffer[150],isobuffer[151]);
	readraw(device,dt,1,(unsigned short *)isobuffer);
	
	unsigned long res = charstoint(isobuffer[2],isobuffer[3],isobuffer[4],isobuffer[5]);
	if(path[0]==0){
		return res;
	}
	int pathpointer = 0;
	char tot = 1;
	int tcnt = 0;
	
	nextpathelem:
	tcnt = 0;
	isonameloc = pathpointer;
	for(int i = 0 ; i < 30 ; i++){
		char deze = path[pathpointer++];
		if(deze==0x00||deze=='/'){
			pathpart[i]=0;
			tcnt = i;
			break;
		}
		pathpart[i]=deze;
	}
	
	int found = 0;
	int i = 0;
	for(int y = 0 ; y < 10 ; y++){
		char ttutA = isobuffer[i+0];
		char ttutB = isobuffer[i+1];
		char ttutC = isobuffer[i+7];
		if(tot==ttutC){
//			putc('\"');
//			for(int z = 0 ; z < ttutA ; z++){
//				putc(buffer[i+8+z]);
//			}
//			putc('\"');
			if(tcnt==ttutA){
				found = 1;
				for(int z = 0 ; z < ttutA ; z++){
					char A = isobuffer[i+8+z];
					char B = pathpart[z];
					if(A!=B){
						found = 0;
					}
				}
				if(found==1){
					tot = y+1;
					res = charstoint(isobuffer[i+2],isobuffer[i+3],isobuffer[i+4],isobuffer[i+5]);
					selfloor = tot;
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
	}else if(path[pathpointer-1]==0x00){
		return res;
	}else{
		return 0;
	}
	
	foundyay:
	return res;
}

void iso_9660_dir(Device *device,char* path,char *buffer){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	int target = iso_9660_target(device,path);
	if(target!=0){
		int tor = 0;
		int i = 0;
		int gz = 0;
		for(int y = 0 ; y < 10 ; y++){
			char ttutA = isobuffer[i+0];
			char ttutB = isobuffer[i+1];
			char ttutC = isobuffer[i+7];
			if(selfloor==ttutC){
				if(gz){
					buffer[tor++] = ';';
				}
				if(isobuffer[i+8]!=0x00){
					for(int z = 0 ; z < ttutA ; z++){
						buffer[tor++] = isobuffer[i+8+z];
					}
					gz++;
				}
			}
			int z = ttutA+ttutB+8;
			if(z%2!=0){
				z++;
			}
			i += z;
		}
		readraw(device,target,1,(unsigned short *)isobuffer);
		for(i = 0 ; i < 1000 ; i++){
			int t = 2;
			if(isobuffer[i]==';'&&isobuffer[i+1]=='1'){
				int fnd = 0;
				for(int z = 1 ; z < 30 ; z++){
					if(isobuffer[i-z]==t){
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
						buffer[tor++] = isobuffer[(i-t)+z];
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

void iso_9660_read(Device *device,char* path,char *buffer){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	int target = iso_9660_target(device,path);
	if(target!=0){
		int tor = 0;
		int i = 0;
		int gz = 0;
		readraw(device,target,1,(unsigned short *)isobuffer);
		unsigned char* fname = (unsigned char*)(path+isonameloc);
		for(i = 0 ; i < 1000 ; i++){
			int t = 2;
			if(isobuffer[i]==';'&&isobuffer[i+1]=='1'){
				int fnd = 0;
				for(int z = 1 ; z < 30 ; z++){
					if(isobuffer[i-z]==t){
						fnd = z;
						break;
					}
					t++;
				}
				if(fnd){
					//t -= 2;
					int w = 0;
					gz = 1;
					for(int z = 2 ; z < t ; z++){
						if(fname[w++]!=isobuffer[(i-t)+z]){
							gz = 0;
						}
					}
					if(gz){
						int tocbse = (((i-t)+1)-32);
						int btok = tocbse+6;
						int bt0k = tocbse+14;
						unsigned long lba = charstoint(isobuffer[btok],isobuffer[btok+1],isobuffer[btok+2],isobuffer[btok+3]);
						unsigned long cnt = (charstoint(isobuffer[bt0k],isobuffer[bt0k+1],isobuffer[bt0k+2],isobuffer[bt0k+3])/device->arg5)+1;
						readraw(device,lba,cnt,(unsigned short *)buffer);
						return;
					}
				}
			}
		}
		buffer[tor++] = 0x00;
	}else{
		buffer[0]=0x00;
	}
}
