#include "../kernel.h"
#define SFS_MAX_FILE_NAME 12
#define SFS_FILE_TABLE_ENTRIES 32

//
// Filesystem created by Alexandros de Regt
// 

typedef struct {
	// this code is the bootjump which should make the sector able to boot
	unsigned char 		bootjmp[3];
	// this signature proves it is a SFS sector
	unsigned char		signature; // always 0xCD
	// this versionnumber should be 1
	unsigned char		version; // always 1
	// sectorsize of medium
	unsigned short		sectorsize;
	// offset to first sectortable
	unsigned short		offset_first_sectortable;
	// sectors per sectortable 
	unsigned char		sectortablesize;
	// bootcode
	unsigned char		bootcode[502];
}__attribute__((packed)) SFSBootsector;

typedef struct{
	// filename
	unsigned char filename[SFS_MAX_FILE_NAME];
	// fileid
	unsigned char fileid;
	// filesize in last sector
	unsigned short filesize_lastsector;
	// flags
	unsigned char flags;
}__attribute__((packed)) SFSFileEntry;

typedef struct{
	SFSFileEntry entries[SFS_FILE_TABLE_ENTRIES];
}__attribute__((packed , aligned (0x100))) SFSDirectory;

SFSBootsector bootsector;
SFSDirectory filetable;
unsigned char pathbuffer[SFS_MAX_FILE_NAME];
unsigned char tabletable[512];

void sfs_get_detail(char* path){
	int pathpointer = 0;
	if(path[0]==0x00){
		goto end;
	}
	try_again:
	// clear buffer
	for(int i = 0 ; i < SFS_MAX_FILE_NAME ; i++){
		pathbuffer[i] = 0x00;
	}
	// fill with filename
	char has_dot = 0;
	for(int i = 0 ; i < SFS_MAX_FILE_NAME ; i++){
		char deze = path[pathpointer];
		if(deze=='/'||deze==0x00){
			break;
		}
		if(deze=='.'){
			has_dot = 1;
		}
		pathbuffer[i] = deze;
		pathpointer++;
	}
	// does filename contains dot? then we are ready
	if(has_dot){
		goto end;
	}
	// does the name exists?
	unsigned char nameexists = 0;
	for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
		SFSFileEntry fe = filetable.entries[i];
		unsigned char te = 1;
		for(int y = 0 ; y < SFS_MAX_FILE_NAME ; y++){
			if(fe.filename[y]!=pathbuffer[y]){
				te = 0;
			}
		}
		if(te){
			nameexists = 1;
		}
	}
	if(nameexists==0){
		goto end;
	}
	// must we try again?
	if(path[pathpointer]=='/'){
		pathpointer++;
		goto try_again;
	}
	if(path[pathpointer]==0x00){
		goto end;
	}
	goto try_again;
	end:
	return;
}

void sfs_dir(Device *device,char* path,char *buffer){
	//void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	sfs_get_detail(path);
	int counter = 0;
	if(device->arg5!=512){
		debugf("Illegal sector size");for(;;);
	}
	for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
		SFSFileEntry fe = filetable.entries[i];
		int t = 0;
		for(int y = 0 ; y < SFS_MAX_FILE_NAME ; y++){
			if(fe.filename[y]!=0x00){
				putc(fe.filename[y]);
				buffer[counter++] = fe.filename[y];
				t = 1;
			}
		}
		if(t){
			buffer[counter++] = ';';
		}
	}
	if(counter>0){
		buffer[counter-1] = 0x00;
	}
}

char sfs_new_file(Device *device,char* path){
	void* (*writeraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->writeRawSector;
	sfs_get_detail(path);
	// which number exists?
	char number = 2;
	while(1){
		char found = 0;
		for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
			if(filetable.entries[i].fileid==number){
				found = 1;
			}
		}
		if(found==1){
			number++;
			found = 0;
		}else{
			break;
		}
	}
	for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
		if(filetable.entries[i].fileid==0){
			filetable.entries[i].fileid = number;
			for(int t = 0 ; t < SFS_MAX_FILE_NAME ; t++){
				filetable.entries[i].filename[t] = pathbuffer[t];
			}
			break;
		}
	}
	writeraw(device,bootsector.offset_first_sectortable + bootsector.sectortablesize,1,(unsigned short*)((unsigned long)&filetable));
	return number;
}

char sfs_write(Device *device,char* path,char *buffer,int size){
	void* (*writeraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->writeRawSector;

	//
	// First, calculate size in sectors
	int sectorsize = 1;
	int calcsect = 0;
	int physsecsize = 512;
	againcalc:
	calcsect = sectorsize*physsecsize;
	if(calcsect>size){
		goto finishedcalc;
	}
	sectorsize++;
	goto againcalc;
	finishedcalc:

	//
	// then, get current file info
	sfs_get_detail(path);
	unsigned char nameexists = 0;
	for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
		SFSFileEntry fe = filetable.entries[i];
		unsigned char te = 1;
		for(int y = 0 ; y < SFS_MAX_FILE_NAME ; y++){
			if(fe.filename[y]!=pathbuffer[y]){
				te = 0;
			}
		}
		if(te){
			nameexists = fe.fileid;
			goto acknowledge;
		}
	}
	if(nameexists==0){
		nameexists = sfs_new_file(device,path);
	}
	acknowledge:
	//
	// then: remove previous data
	for(unsigned long i = 0 ; i < 512 ; i++){
		if(tabletable[i]==nameexists){
			tabletable[i] = 0;
		}
	}
	//
	// then: fill system with new data
	int tmpsect = sectorsize;
	for(unsigned long i = 3 ; i < 512 ; i++){
		if(tabletable[i]==0){
			tabletable[i] = nameexists;
			tmpsect--;
			if(tmpsect==0){
				break;
			}
		}
	}
	writeraw(device,bootsector.offset_first_sectortable,1,(unsigned short *)&tabletable);
	unsigned char sectoroffsettemp = 0;
	for(unsigned long i = 0 ; i < 512 ; i++){
		if(tabletable[i]==nameexists){
			writeraw(device,i,1,(unsigned short*)(buffer+sectoroffsettemp));
			sectoroffsettemp += 512;
		}
	}
	return 1;
}

char sfs_exists(Device *device,char* path){
	//void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	if(device->arg5!=512){
		debugf("Illegal size\n");for(;;);
	}
	sfs_get_detail(path);
	unsigned char nameexists = 0;
	for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
		SFSFileEntry fe = filetable.entries[i];
		unsigned char te = 1;
		for(int y = 0 ; y < SFS_MAX_FILE_NAME ; y++){
			if(fe.filename[y]!=pathbuffer[y]){
				te = 0;
			}
		}
		if(te){
			nameexists = fe.fileid;
			return 1;
		}
	}
	if(nameexists==0){
		return 0;
	}

	return 1;
}

unsigned char sfs_read(Device *device,char* path,char *buffer){
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	sfs_get_detail(path);
	unsigned char nameexists = 0;
	for(int i = 0 ; i < SFS_FILE_TABLE_ENTRIES ; i++){
		SFSFileEntry fe = filetable.entries[i];
		unsigned char te = 1;
		for(int y = 0 ; y < SFS_MAX_FILE_NAME ; y++){
			if(fe.filename[y]!=pathbuffer[y]){
				te = 0;
			}
		}
		if(te){
			nameexists = fe.fileid;
			goto found;
		}
	}
	if(nameexists==0){
		return 0 ;
	}

	unsigned int offset = 0;
	found:
	offset = 0;
	for(unsigned long i = 0 ; i < 512 ; i++){
		if(tabletable[i]==nameexists){
			debugf("SFS: read %x [%x] \n",i,device->arg2+i);
			unsigned long dir = ((unsigned long)buffer)+offset;
			readraw(device,i,1,(unsigned short*)dir);
			offset += 512;
		}
	}
	return 1;
}

void initialiseSFS(Device *device){
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	debugf("[SFS] SanderOSUSB File System (SFS) driver activated\n");
	debugf("[SFS] Read raw sector code located at %x \n",device->readRawSector);
	unsigned char* bootsecloc = (unsigned char*)&bootsector;
	readraw(device,0,1,(unsigned short*)bootsecloc);
	if(bootsector.signature!=0xCD){
		debugf("[SFS] Invalid SFS bootsector\n");
		return;
	}
	if(bootsector.version!=1){
		debugf("[SFS] Invalid SFS versionnumber: %x \n",bootsector.version);
		return;
	}
	debugf("[SFS] Sectorsize: %x \n",bootsector.sectorsize);
	debugf("[SFS] Offset first sectortable: %x \n",bootsector.offset_first_sectortable);
	debugf("[SFS] Sector table size: %x \n",bootsector.sectortablesize);

	unsigned char* filetableloc = (unsigned char*)&filetable;
	unsigned char* tabletableloc = (unsigned char*)&tabletable;
	unsigned long t0 = 0;
	unsigned long t1 = 0;
	t0 += bootsector.offset_first_sectortable + bootsector.sectortablesize;
	t1 += bootsector.offset_first_sectortable;
	readraw(device,t0,1,(unsigned short*)filetableloc);
	readraw(device,t1,1,(unsigned short*)tabletableloc);
	
	device->dir = (unsigned long)&sfs_dir;
	device->readFile = (unsigned long)&sfs_read;
	device->existsFile = (unsigned long)&sfs_exists;
	device->writeFile = (unsigned long)&sfs_write;
	device->newFile = (unsigned long)&sfs_new_file;

	debugf("[SFS] readSector: %x dir: %x \n",device->readRawSector,device->dir);

}