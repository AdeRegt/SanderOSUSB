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

typedef struct{//					WITH DATA ON LIVE DISK
	unsigned short typeandpremissions;	//	0xed 0x41
	unsigned short userid;			//	0x00 0x00
	unsigned long losize;			//	0x00 0x10 0x00 0x00
	unsigned long accesstime;		//	0xeb 0x32 0xaf 0x5d
	unsigned long creationtime;		//	0xe8 0x32 0xaf 0x5d
	unsigned long lastmodified;		//	0xe8 0x32 0xaf 0x5d
	unsigned long delitiontime;		//	0x00 0x00 0x00 0x00
	unsigned short groupid;			//	0x00 0x00
	unsigned short counthardlinks;		//	0x19 0x00
	unsigned long countharddisksectors;	//	0x08 0x00 0x00 0x00
	unsigned long flags;			//	0x00 0x00 0x08 0x00
	unsigned long osspecval;		//	0x6a 0x01 0x00 0x00
	unsigned long directp[12];		// [0]	0x0a 0xf3 0x01 0x00 
						// [1]	0x04 0x00 0x00 0x00
	unsigned long singlipd;
	unsigned long doubld;
	unsigned long tripple;
	unsigned long generation;
	unsigned long ext1;
	unsigned long ext2;
	unsigned long blckadrsfrg;
	unsigned char reserved[12+0x80];
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
	
	unsigned short* efi = (unsigned short*) malloc(device->arg5);
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
		
		// blockdevice info ophalen
		unsigned char* ext = (unsigned char*) 0x1500;
		readraw(device, offsetblockgroupsectphys, 1, (unsigned short *)ext);
		EXT2BlockGroupDesc* et = (EXT2BlockGroupDesc*) ext;
		unsigned short* grouproot = (unsigned short*) 0x1500+512;
		printf("[EXT2] Group0, inodetablestart at %x and %x dirs declared with sizeofinode of %x \n",et->inodetablestart*blocksector,et->directoriesingroup,superblock->sizeofinode);
		readraw(device, et->inodetablestart*blocksector, 1, grouproot);
		if(superblock->sizeofinode!=0x100){
			printf("[EXT2] unexpected inode size!!!\n");
		}
		//
		// root node vinden
		EXT2Inode* lst = (EXT2Inode*) &(((EXT2Inode*) grouproot)[1]);
		unsigned long rootdirtype = lst->typeandpremissions;
		rootdirtype = rootdirtype >> 12;
		if(rootdirtype==4){
			printf("[EXT2] Root dir found!\n");
			for(int i = 0 ; i < 12 ; i++){
				printf("[EXT2] DRPNT: %x \n",lst->directp[i]);
			}
			if(lst->directp[0]==0x1f30a){
				printf("[EXT2] ACK\n");
			}
			unsigned long directp = (lst->directp[0])*blocksector;//+superblock->blocksforsu;
			unsigned short* dirlist = (unsigned short*) 0x1500;
			readraw(device, directp, 1, dirlist);
			
		}else{
			printf("[EXT2] Root dir NOT found! [ %x : %x ] \n",rootdirtype,lst->typeandpremissions);
		}
	}else{
		printf("[EXT2] Illegal checksum %x \n",superblock->checksum);
	}
}
