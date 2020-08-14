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
	selfloor = 1;

	//printf("Te zoeken path= %s \n",path);

	//
	// analyseren pathgegevens
	// / replacen met 0x0D
	// opzoeken of . bestaat
	//
	int pathlengte = strlen(path);
	int paths = 1;
	char is_bestand = 0;
	for(int i = 0 ; i < pathlengte ; i++){
		if(path[i]=='/'){
			path[i] = 0x0d;
			paths++;
		}
		if(path[i]=='.'){
			is_bestand = 1;
		}
	}
	//printf("Er zijn %x pathelementen\n",paths);

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

	char pathchunk[20];
	int pathsel = 0; // pathchunckcount
	int ipath = 0; // pathchunksel
	char deze = 0;
	int boomdiepte = 1;
	memset(pathchunk,20,0);
	for(int i = 0 ; i < (paths-(is_bestand?1:0)) ; i++){
		memset(pathchunk,20,0);
		ipath = 0;
		kopieernogeen:
		deze = path[pathsel++];
		if(!(deze==0x00||deze==0x0D)){
			pathchunk[ipath++] = deze;
			goto kopieernogeen;
		}
		pathchunk[ipath] = 0x00;
		//printf("Chunk [%s] word nu behandeld \n",pathchunk);

		// door alle directories lopen van actuele boom
		unsigned char entrytextlength = 0;
		unsigned char entrytotallength = 0;
		unsigned char entrytree = 0;
		int entrypointer = 0;
		int edept = 0;
		nogmaals:
		entrytextlength = isobuffer[entrypointer+0];
		entrytotallength = isobuffer[entrypointer+1];
		entrytree = isobuffer[entrypointer+7];
		if(entrytree==boomdiepte&&entrytextlength==ipath){
			char found = 1;
			for(int t = 0 ; t < entrytextlength ; t++){
				if(isobuffer[entrypointer+8+t]!=pathchunk[t]){
					found = 0;
				}
			}
			if(found){
				boomdiepte = edept+1;
				res = charstoint(isobuffer[entrypointer+2],isobuffer[entrypointer+3],isobuffer[entrypointer+4],isobuffer[entrypointer+5]);
				selfloor = boomdiepte;
				continue;
			}
		}
		int z = entrytextlength+entrytotallength+8;
		if(z%2!=0){
			z++;
		}
		entrypointer += z;
		edept++;
		goto nogmaals;
	}
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

char iso_9660_exists(Device *device,char* path){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	int target = iso_9660_target(device,path);
	if(target!=0){
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
						return 1;
					}
				}
			}
		}
	}
	return 0;
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
						for(unsigned int q = 0 ; q < cnt ; q++){
							readraw(device,lba+q,1,(unsigned short *)(buffer+(device->arg5*q)));
						}
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
