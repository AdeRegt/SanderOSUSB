#include "../kernel.h"

typedef struct fat_extBS_32{
	//extended fat32 stuff
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];
 
}__attribute__((packed)) fat_extBS_32_t;
 
typedef struct fat_extBS_16{
	//extended fat12 and fat16 stuff
	unsigned char		bios_drive_num;
	unsigned char		reserved1;
	unsigned char		boot_signature;
	unsigned int		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];
 
}__attribute__((packed)) fat_extBS_16_t;
 
typedef struct fat_BS{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	        bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
 
	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];
 
}__attribute__((packed)) fat_BS_t;

typedef struct DirectoryEntry{
	unsigned char name[11];
	unsigned char attrib;
	unsigned char userattrib;
	unsigned char undelete;
	unsigned short createtime;
	unsigned short createdate;
	unsigned short accessdate;
	unsigned short clusterhigh; // 00 00 
	unsigned short modifiedtime;
	unsigned short modifieddate; 
	unsigned short clusterlow; // 66 13
	unsigned long filesize;
 
} __attribute__ ((packed)) fat_dir_t;

fat_BS_t* fat_boot;
unsigned long first_data_sector;

unsigned long fat_get_first_sector_of_cluster(Device *device){
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	unsigned short* rxbuffer = (unsigned short*) malloc(512);
	readraw(device,0,1,rxbuffer); 
	fat_boot = (fat_BS_t*) rxbuffer;
	fat_extBS_32_t* fat_boot_ext_32 = (fat_extBS_32_t*) fat_boot->extended_section;
	
	unsigned long total_sectors 	= (fat_boot->total_sectors_16 == 0)? fat_boot->total_sectors_32 : fat_boot->total_sectors_16;
	unsigned long fat_size 		= (fat_boot->table_size_16 == 0)? fat_boot_ext_32->table_size_32 : fat_boot->table_size_16;
	unsigned long root_dir_sectors 	= ((fat_boot->root_entry_count * 32) + (fat_boot->bytes_per_sector - 1)) / fat_boot->bytes_per_sector;
	first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors;
	unsigned long data_sectors 	= total_sectors - (fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors);
	unsigned long total_clusters 	= data_sectors / fat_boot->sectors_per_cluster;
	
	unsigned long first_sector_of_cluster = 0;
	if(total_clusters < 4085){
		first_sector_of_cluster = 19;
	}else if(total_clusters < 65525){
		first_sector_of_cluster = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_boot->table_size_16);
	}else if (total_clusters < 268435445){
		unsigned long root_cluster_32 = fat_boot_ext_32->root_cluster;
		first_sector_of_cluster = ((root_cluster_32 - 2) * fat_boot->sectors_per_cluster) + first_data_sector;
	}
	free(rxbuffer);
	return first_sector_of_cluster;
}

unsigned long fat_get_first_sector_of_file(Device *device,char *path){
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned char *) = (void*)device->readRawSector;

	unsigned long first_sector_of_cluster = fat_get_first_sector_of_cluster(device);
	
	unsigned char* fatbuffer = (unsigned char*) malloc(512);
	readraw(device,first_sector_of_cluster,1,fatbuffer); 
	unsigned long pathoffset = 0;
	unsigned long pathfileof = 0;
	unsigned char filename[11];
	
	//
	// lookup path
	while(1){
		// clear buffer
		for(int i = 0 ; i < 11 ; i++){
			filename[i] = 0x00;
		}
		// fill buffer with new word
		unsigned char erstw = path[pathoffset];
		if(erstw==0x00){
			break;
		}
		
		for(int i = 0 ; i < 11 ; i++){
			unsigned char deze = path[pathoffset++];
			if(deze=='/'){
				break;
			}
			if(deze==0x00){
				pathoffset--;
				break;
			}
			filename[pathfileof++] = deze;
		}
		
		unsigned long offset = 0;
		unsigned long newsect = 0;
		int marx = 0;
		while(1){
			fat_dir_t* currentdir = (fat_dir_t*) (fatbuffer + offset);
			offset += sizeof(fat_dir_t);
			marx++;
			if(marx>50){break;}
			unsigned char eersteteken = currentdir->name[0];
			if(eersteteken==0x00){
				continue;
			}
			if((eersteteken=='A'&&currentdir->name[2]==0x00)||eersteteken==0xE5){
				continue;
			}
			unsigned long sigma = 0;
			unsigned long yotta = 1;
			for(int i = 0 ; i < 11 ; i++){
				if(currentdir->name[i]!=0x00&&currentdir->name[i]!=' '){
					if(currentdir->name[i]!=filename[sigma++]){
						yotta = 0;
					}
				}
				if(i==7&&currentdir->attrib==0x20&&filename[sigma++]!='.'){
					yotta = 0;
				}
				if(filename[sigma]==0){
					break;
				}
			}
			if(yotta){
				newsect = ((currentdir->clusterhigh << 8) &0xFFFF0000) | (currentdir->clusterlow & 0xFFFF);
				break;
			}
		}
		if(newsect){
			first_sector_of_cluster = ((newsect - 2) * fat_boot->sectors_per_cluster) + first_data_sector;
			readraw(device,first_sector_of_cluster,1,fatbuffer);
		}else{
			free(fatbuffer);
			return 0;
		}
		
	}
	free(fatbuffer);
	return first_sector_of_cluster;
}

char fat_exists(Device *device,char *path){
	unsigned long dk1 = fat_get_first_sector_of_file(device,path);
	return dk1>0;
}

void fat_write(Device *device,char* path,char *buffer,int size){}

void fat_create(Device *device,char* path,char *buffer,int size){}

char fat_read(Device *device,char* path,char *buffer){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	unsigned long first_sector_of_cluster = fat_get_first_sector_of_file(device,path);
	if(first_sector_of_cluster==0){
		return 0;
	}
	readraw(device,first_sector_of_cluster,1,(unsigned short*)buffer);
	return 1;
}

void fat_dir(Device *device,char* path,char *buffer){
	//atapi_read_raw(Device *dev,unsigned long lba,unsigned char count,unsigned short *location)
	void* (*readraw)(Device *,unsigned long,unsigned char,char *) = (void*)device->readRawSector;

	unsigned long first_sector_of_cluster = fat_get_first_sector_of_file(device,path);
	if(first_sector_of_cluster==0){
		return;
	}

	char *fatbuffer = (char*) malloc(512);

	readraw(device,first_sector_of_cluster,1,fatbuffer);
	
	//
	// print files
	unsigned long offset = 0;
	unsigned long bufofs = 0;
	while(1){
		fat_dir_t* currentdir = (fat_dir_t*) (fatbuffer + offset);
		offset += sizeof(fat_dir_t);
		unsigned char eersteteken = currentdir->name[0];
		if(eersteteken==0x00){
			break;
		}
		if((eersteteken=='A'&&currentdir->name[2]==0x00)||eersteteken==0xE5){
			continue;
		}
		for(int i = 0 ; i < 11 ; i++){
			if(currentdir->name[i]!=0x00&&currentdir->name[i]!=0x20){
				buffer[bufofs++] = currentdir->name[i];
			}
			if(i==7&&currentdir->attrib==0x20){
				buffer[bufofs++] = '.';
			}
		}
		buffer[bufofs++] = ';';
	}
	buffer[bufofs-1] = 0x00;
	buffer[bufofs] = 0x00;
	free(fatbuffer);
}

void initialiseFAT(Device* device){
	int (*readraw)(Device *,unsigned long,unsigned char,unsigned short *) = (void*)device->readRawSector;
	unsigned short* buffer = (unsigned short*) malloc(512);
	int tres = readraw(device,0,1,buffer);
	if(tres==0){
		printf("[FAT] FAIL: cannot read sector\n");
		return;
	}
	fat_BS_t* fat_boot = (fat_BS_t*) buffer;
	fat_extBS_32_t* fat_boot_ext_32 = (fat_extBS_32_t*) fat_boot->extended_section;
	printf("[FAT] FAT detected!\n");
	printf("[FAT] OEM-name: \"");
	for(int i = 0 ; i < 8 ; i++){
		printf("%c",fat_boot->oem_name[i]);
	}
	printf("\"\n");
	unsigned long total_sectors 	= (fat_boot->total_sectors_16 == 0)? fat_boot->total_sectors_32 : fat_boot->total_sectors_16;
	unsigned long fat_size 		= (fat_boot->table_size_16 == 0)? fat_boot_ext_32->table_size_32 : fat_boot->table_size_16;
	unsigned long root_dir_sectors 	= ((fat_boot->root_entry_count * 32) + (fat_boot->bytes_per_sector - 1)) / fat_boot->bytes_per_sector;
	unsigned long first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors;
	unsigned long first_fat_sector 	= fat_boot->reserved_sector_count;
	unsigned long data_sectors 	= total_sectors - (fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors);
	unsigned long total_clusters 	= data_sectors / fat_boot->sectors_per_cluster;
	
	printf("[FAT] total sectors %x \n",total_sectors);
	printf("[FAT] fatsize %x \n",fat_size);
	printf("[FAT] root dir sectors %x \n",root_dir_sectors);
	printf("[FAT] first data sector %x \n",first_data_sector);
	printf("[FAT] first fat sector %x \n",first_fat_sector);
	printf("[FAT] data sectors %x \n",data_sectors);
	printf("[FAT] total clusters %x \n",total_clusters);
	if(total_clusters < 4085){
		printf("[FAT] FAT-type is FAT12\n");
	}else if(total_clusters < 65525){
		printf("[FAT] FAT-type is FAT16\n");
	}else if (total_clusters < 268435445){
		printf("[FAT] FAT-type is FAT32\n");
	}else{ 
		printf("[FAT] FAT-type is ExFAT\n");
	}
	unsigned long first_sector_of_cluster = fat_get_first_sector_of_cluster(device);
	printf("[FAT] First sector of cluster %x \n",first_sector_of_cluster);
	if(first_sector_of_cluster==0){
		printf("[FAT] FAT-dir is 0. No FAT supported!\n");
		return;
	}
	device->dir	= (unsigned long)&fat_dir;
	device->readFile = (unsigned long)&fat_read;
	device->existsFile = (unsigned long)&fat_exists;
	if(device->writeRawSector){
		printf("[FAT] Enables FAT write function\n");
		device->writeFile = (unsigned long)&fat_write;
		device->newFile = (unsigned long)&fat_create;
	}else{
		printf("[FAT] FAT is readonly\n");
	}
}
