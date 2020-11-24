#include "../kernel.h"

typedef struct{
	unsigned char drive_attr;
	unsigned char chsstr[3];
	unsigned char type;
	unsigned char chsend[3];
	unsigned long lbastart;
	unsigned long seccnt;
}mbr_entry;

//
// Parse MBR or EFI
void detectFilesystemsOnMBR(Device* device){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	unsigned long atmp2 = device->readRawSector;
	unsigned long atmp3 = device->writeRawSector;

	unsigned char* msg = (unsigned char*) 0x1000;
	readraw(device, 0, 1, (unsigned short *)msg);
	unsigned int basex = 0x01BE;
	mbr_entry* mbrs = (mbr_entry*)&msg[basex];
	for(int i = 0 ; i < 4 ; i++){
		printf("[MBRI] MBR %x : lba %x type %x \n",i+1,mbrs[i].lbastart,mbrs[i].type);
		if(mbrs[i].type==0xEE){
			printf("[MBRI] MBR partitiontable declares EFI subsystem\n");
			unsigned char* efi = (unsigned char*) 0x1000+512;
			readraw(device, mbrs[i].lbastart, 1, (unsigned short *)efi);
			if(efi[0]==0x45&&efi[1]==0x46&&efi[2]==0x49&&efi[3]==0x20&&efi[4]==0x50&&efi[5]==0x41&&efi[6]==0x52&&efi[7]==0x54){
				printf("[MBRI] EFI checksum OK\n");
			}else{
				continue;
			}
			readraw(device, mbrs[i].lbastart+1, 1, (unsigned short *)efi);
			for(int z = 0 ; z < 512 ; z = z+128){
				printf("[MBRI] partname \"");
				for(int k = 0 ; k < 72 ; k++){
					char deze = efi[z+56+k];
					if(deze!=0x00){
						printf("%c",deze);
					}
				}
				unsigned char tA = efi[z+32+0];
				unsigned char tB = efi[z+32+1];
				unsigned char tC = efi[z+32+2];
				unsigned char tD = efi[z+32+3];
				unsigned char xt[4];
				xt[0] = tA;
				xt[1] = tB;
				xt[2] = tC;
				xt[3] = tD;
				unsigned long xy = ((unsigned long*)&xt)[0];
				printf("\" type %x%x %x%x %x%x %x%x \n",efi[z+8+0],efi[z+8+1],efi[z+8+2],efi[z+8+3],efi[z+8+4],efi[z+8+5],efi[z+8+6],efi[z+8+7]);
				if((efi[z+8+6]==0xC9&&efi[z+8+7]==0x3B)||(efi[z+8+6]==0x99&&efi[z+8+7]==0xC7)){
					printf("[MBRI] EFI: FAT-formated partition found at %x! \n",xy);
					
					Device *regdev = getNextFreeDevice();
		
					regdev->readRawSector 	= device->readRawSector;
					
					regdev->arg1 = device->arg1;
					regdev->arg2 = xy;
					regdev->arg3 = device->arg3;
					regdev->arg4 = device->arg4;
					regdev->arg5 = device->arg5;
					initialiseFAT(regdev);
				}else if(efi[z+8+6]==0x7D&&efi[z+8+7]==0xE4){
					printf("[MBRI] EFI: Linux native filesystem detected at %x !\n",xy);
					
					Device *regdev = getNextFreeDevice();
		
					regdev->readRawSector 	= device->readRawSector;
					
					regdev->arg1 = device->arg1;
					regdev->arg2 = xy;
					regdev->arg3 = device->arg3;
					regdev->arg4 = device->arg4;
					regdev->arg5 = device->arg5;
					initialiseExt2(regdev);
				}
			}
		}else if(mbrs[i].type==0x83){
			printf("[MBRI] Linux native filesystem detected\n");
			Device *regdev = getNextFreeDevice();
		
			regdev->readRawSector 	= device->readRawSector;
			
			regdev->arg1 = device->arg1;
			regdev->arg2 = mbrs[i].lbastart;
			regdev->arg3 = device->arg3;
			regdev->arg4 = device->arg4;
			regdev->arg5 = device->arg5;
			initialiseExt2(regdev);
		}else if(mbrs[i].type==0x0E||mbrs[i].type==0x0B||mbrs[i].type==0x0C){
			printf("[MBRI] FAT filesystem detected\n");
			Device *regdev = getNextFreeDevice();
		
			regdev->readRawSector 	= atmp2;
			
			regdev->arg1 = device->arg1;
			regdev->arg2 = mbrs[i].lbastart;
			regdev->arg3 = device->arg3;
			regdev->arg4 = device->arg4;
			regdev->arg5 = device->arg5;
			initialiseFAT(regdev);
		}else if(mbrs[i].type==0xCD){
			printf("[MBRI] SFS filesystem detected\n");
			Device *regdev = getNextFreeDevice();
		
			regdev->readRawSector 	= atmp2;
			regdev->writeRawSector 	= atmp3;
			regdev->arg1 = device->arg1;
			regdev->arg2 = mbrs[i].lbastart;
			regdev->arg3 = device->arg3;
			regdev->arg4 = device->arg4;
			regdev->arg5 = device->arg5;

			initialiseSFS(regdev);
		}
		basex += 16;
	}
}
