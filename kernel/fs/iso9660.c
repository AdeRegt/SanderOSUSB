#include "../kernel.h"

void iso_9660_dir(Device *device,char* path,char *buffer){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	//
	// eerst opzoeken van de eerste sector
	//
	readraw(device,1,1,(unsigned short *)buffer);
}
