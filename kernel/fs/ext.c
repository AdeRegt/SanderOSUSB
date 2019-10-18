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
	// extra ones
	unsigned long firstnonreserved;
	unsigned short sizeofinode;
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
	
	unsigned char* efi = (unsigned char*) malloc(device->arg5);
	printf("[EXT2] PARSING EXT2\n");
	unsigned long superblockid = 2;
	readraw(device, superblockid, 1, (unsigned short *)efi);
	EXT2Super *superblock = (EXT2Super*) efi;
	if(superblock->checksum==0xEF53){
		printf("[EXT2] Detected by signature!!\n");
		unsigned long blocksize = 1024 << superblock->blocksize;
		printf("[EXT2] Have blocksize %x \n",blocksize);
		unsigned long blocksector = blocksize/512;
		printf("[EXT2] One block has %x sectors \n",blocksector);
		unsigned long offsetblockgroup = 1;
		if(blocksize==1024){
			offsetblockgroup = 2;
		}
		unsigned long inodecount = superblock->total_inodes;
		printf("[EXT2] We have a inode count of %x \n",inodecount);
		unsigned long offsetblockgroupsect = offsetblockgroup*blocksector;
		unsigned long offsetblockgroupsectphys = offsetblockgroupsect;
		printf("[EXT2] Offset to blockgrouptable is %x (virt %x : phy %x)\n",offsetblockgroup,offsetblockgroupsect,offsetblockgroupsectphys);
		printf("[EXT2] First non-reserved inode=%x sizeof inode=%x \n",superblock->firstnonreserved,superblock->sizeofinode);
		//
		// blockdevice info ophalen
		unsigned char* ext = (unsigned char*) 0x1500;
		readraw(device, offsetblockgroupsectphys, 1, (unsigned short *)ext);
		EXT2BlockGroupDesc* et = (EXT2BlockGroupDesc*) ext;
		unsigned char* grouproot = (unsigned char*) 0x1500+512;
		printf("[EXT2] Group0, inodetablestart at %x \n",et->inodetablestart*blocksector);
		unsigned long offset = 0;
		for(int i = 0 ; i < blocksector*5 ; i++){
			readraw(device, ((et->inodetablestart)*blocksector)+i, 1, (unsigned short *)grouproot+offset);
			offset += 512;
		}
		//
		// root node vinden
		EXT2Inode* lst = (EXT2Inode*) grouproot + (superblock->sizeofinode*1);
		unsigned long rootdirtype = lst->typeandpremissions;
		rootdirtype = rootdirtype >> 12;
		if(rootdirtype==4){
			printf("[EXT2] Root dir found!\n");
			unsigned long directp = lst->directp[0];
			unsigned char* dirlist = (unsigned char*) 0x1500;
			readraw(device, directp, 1, (unsigned short *)dirlist);
			for(int i = 0 ; i < 512 ; i++){
				printf("%c",dirlist[i]);
			}
		}
	}
}
