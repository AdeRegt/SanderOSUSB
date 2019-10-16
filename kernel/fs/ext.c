#include "../kernel.h"

//
// 


typedef struct{
	unsigned long total_inodes;
	unsigned long total_blocks;
	unsigned long blocksforsu;
	unsigned long unallockblocks;
	unsigned long unallockinodes;
	unsigned long blocknrsprblck;
	unsigned long blocksize;
	unsigned long fragmentsize;
	unsigned long numberofblocksineachblockgroup;
	unsigned long numberoffragmentsineachblockgroup;
	unsigned long numberofinodesineachblockgroup;
	unsigned long lastmounttime;
	unsigned long lastwrittentime;
	unsigned short mounttimes;
	unsigned short mountconsistance;
	unsigned short checksum; // 0xef53
	unsigned short filesystemstate;
	unsigned short whattodowhenerror;
	unsigned short minorportionversion;
	unsigned long lastconsistancy;
	unsigned long interval;
	unsigned long OS;
	unsigned long majorportionversion;
	unsigned short userID;
	unsigned short groupID;
}EXT2Super;

typedef struct{
	unsigned long blockusagebitmap;
	unsigned long inodeusagebitmap;
	unsigned long inodetablestart;
	unsigned short unallocblockingroup;
	unsigned short unallocinodeingroup;
	unsigned short directoriesingroup;
	unsigned char unused[13];
}EXT2BlockGroupDesc;

typedef struct{
	unsigned short typeandpremissions;
	unsigned short userid;
	unsigned long losize;
	unsigned long accesstime;
	unsigned long creationtime;
	unsigned long lastmodified;
	unsigned long delitiontime;
	unsigned short groupid;
	unsigned short counthardlinks;
	unsigned long countharddisksectors;
	unsigned long flags;
	unsigned long osspecval;
	unsigned long directp[12];
	unsigned long singlipd;
	unsigned long doubld;
	unsigned long tripple;
	unsigned long generation;
	unsigned long ext1;
	unsigned long ext2;
	unsigned long blckadrsfrg;
	unsigned char reserved[12];
}EXT2Inode;

typedef struct{
	unsigned long inodenummer;
	unsigned short recordlengte;
	unsigned char naamlengte;
	unsigned char type;
	unsigned char naam[255];
}EXT2Directory;

void initialiseExt2(Device* device){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	
	unsigned char* efi = (unsigned char*) 0x1000+512;
	readraw(device, 0, 1, (unsigned short *)efi);
	printf("[EXT2] PARSING EXT2\n");
	unsigned long superblockid = 2;
	readraw(device, superblockid, 1, (unsigned short *)efi);
	EXT2Super *tar = (EXT2Super*) efi;
	if(tar->checksum==0xEF53){
		printf("[EXT2] Detected by signature!!\n");
		unsigned long blocksize = 1024 << tar->blocksize;
		printf("[EXT2] Have blocksize %x \n",blocksize);
		unsigned long blocksector = blocksize/512;
		printf("[EXT2] One block has %x sectors \n",blocksector);
		unsigned long offsetblockgroup = 1;
		if(blocksize==1024){
			offsetblockgroup = 2;
		}
		unsigned long inodecount = tar->total_inodes;
		printf("[EXT2] We have a inode count of %x \n",inodecount);
		unsigned long offsetblockgroupsect = offsetblockgroup*blocksector;
		unsigned long offsetblockgroupsectphys = offsetblockgroupsect;
		printf("[EXT2] Offset to blockgrouptable is %x (virt %x : phy %x)\n",offsetblockgroup,offsetblockgroupsect,offsetblockgroupsectphys);
		unsigned char* ext = (unsigned char*) 0x1000+1024;
		readraw(device, offsetblockgroupsectphys, 1, (unsigned short *)ext);
		EXT2BlockGroupDesc* et = (EXT2BlockGroupDesc*) ext;
		
		//
		// looping blocks
		for(int e = 0 ; e < 3 ; e++){
			printf("[EXT2] #%x : directoriesingroup %x startof list block %x \n",e,et[e].directoriesingroup,et[e].inodetablestart);
			unsigned char* inod = (unsigned char*) 0x1000+1536;
			readraw(device, et[e].inodetablestart*blocksector, 1, (unsigned short *)inod);
			EXT2Inode* inodes = (EXT2Inode*) inod; 
			
			//
			// looping INodes
			for(int u = 0 ; u < (512/sizeof(EXT2Inode)) ; u++){
				unsigned long inodetypes = inodes[u].typeandpremissions >> 12;
				if(inodetypes!=0){
					printf("[EXT2] INode%x: type:%x \n",u,inodetypes);
					if(inodetypes==0x8){
						printf("[EXT2] INode%x is an regular file\n",u);
					}else if(inodetypes==0x4){
						printf("[EXT2] INode%x is an directory\n",u);
						unsigned char* dirtableraw = (unsigned char*) 0x1000+2048;
						printf("[EXT2] Directpointer at %x \n",inodes[u].directp[0]);
						readraw(device, inodes[u].directp[0]*blocksector, 1, (unsigned short *)dirtableraw);
						EXT2Directory* rxr = (EXT2Directory*) dirtableraw;
						for(int w = 0 ; w < 255 ; w++){
							printf("%c",rxr->naam[w]);
						}
						printf("\n");
					}
				}
			}
		}
	}
}
