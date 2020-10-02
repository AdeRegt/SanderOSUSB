#include "../kernel.h"

//
// ATA
//
//

extern void ideirq();
extern void iso_9660_dir();
extern void iso_9660_read();
extern char iso_9660_exists();

volatile int ideXirq = 0;
void irq_ide()
{
	ideXirq = 1;
	printf("IDE: FIRE\n");
	outportb(0x20, 0x20);
	outportb(0xA0, 0x20);
}

void resetIDEFire()
{
	ideXirq = 0;
}

void waitForIDEFire()
{
	resetTicks();
	while (ideXirq == 0)
	{
		if (getTicks() == 5)
		{
			//			printf("IDE: timeout!\n");
			break;
		}
	}
}

typedef struct
{
	unsigned short command;
	unsigned short control;
	unsigned char irq;
	unsigned char slave;
} IDEDevice;

IDEDevice ata1 = {.command = 0x1f0, .control = 0x3f6, .irq = 14, .slave = 0};
IDEDevice ata2 = {.command = 0x1f0, .control = 0x3f6, .irq = 14, .slave = 1};
IDEDevice ata3 = {.command = 0x170, .control = 0x3f6, .irq = 15, .slave = 0};
IDEDevice ata4 = {.command = 0x170, .control = 0x376, .irq = 15, .slave = 1};

char getIDEError(IDEDevice cdromdevice)
{
	unsigned char msg = inportb(cdromdevice.command + 7);
	if ((msg >> 0) & 1)
	{
		if(msg==0xFF)
		{
			printf("IDE: returns FF\n");
			return 1;
		}
		if (msg & 0x80)
		{
			printf("IDE: Bad sector\n");
		}
		if (msg & 0x40)
		{
			printf("IDE: Uncorrectable data\n");
		}
		if (msg & 0x20)
		{
			printf("IDE: No media\n");
		}
		if (msg & 0x10)
		{
			printf("IDE: ID mark not found\n");
		}
		if (msg & 0x08)
		{
			printf("IDE: No media\n");
		}
		if (msg & 0x04)
		{
			printf("IDE: Command aborted\n");
		}
		if (msg & 0x02)
		{
			printf("IDE: Track 0 not found\n");
		}
		if (msg & 0x01)
		{
			printf("IDE: No address mark\n");
		}
		return 1;
	}
	return 0;
}

#define ATA_SR_BSY 0x80	 // Busy
#define ATA_SR_DRDY 0x40 // Drive ready
#define ATA_SR_DF 0x20	 // Drive write fault
#define ATA_SR_DSC 0x10	 // Drive seek complete
#define ATA_SR_DRQ 0x08	 // Data request ready
#define ATA_SR_CORR 0x04 // Corrected data
#define ATA_SR_IDX 0x02	 // Index
#define ATA_SR_ERR 0x01	 // Error

void ide_wait_for_ready(IDEDevice cdromdevice)
{
	unsigned char dev = 0x00;
	resetTicks();
	// wait for atleast 5 times
	for(int i = 0 ; i < 6 ; i++){
		inportb(cdromdevice.command + 7);
	}
	while ((dev = inportb(cdromdevice.command + 7)) & ATA_SR_BSY)
	{
		if(dev & ATA_SR_DRQ)
		{
			break;
		}
		if (dev & ATA_SR_DF)
		{
			printf("IDE: ERROR1\n");
			for (;;)
			return;
		}
		if (dev & ATA_SR_ERR)
		{
			printf("IDE: ERROR2\n");
			for (;;)
				;
		}
		if (getTicks() == 5)
		{
			printf("IDE: TIMEOUT\n");
			break;
		}
	}
}

char read_cmd[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void atapi_eject_raw(IDEDevice cdromdevice)
{
	getIDEError(cdromdevice);

	ide_wait_for_ready(cdromdevice);
	outportb(cdromdevice.command + 6, cdromdevice.slave == 1 ? 0xB0 : 0xA0);
	outportb(cdromdevice.command + 1, 0x00);
	outportb(cdromdevice.command + 7, 0xA0);

	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);

	read_cmd[0] = 0x1B;
	read_cmd[1] = 0x00;
	read_cmd[2] = 0x00;
	read_cmd[3] = 0x00;
	read_cmd[4] = 0x02;
	read_cmd[5] = 0x00;
	read_cmd[6] = 0x00;
	read_cmd[7] = 0x00;
	read_cmd[8] = 0x00;
	read_cmd[9] = 0x00;
	read_cmd[10] = 0x00;
	read_cmd[11] = 0x00;

	resetIDEFire();
	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);
	unsigned short *mdx = (unsigned short *)&read_cmd;
	for (int f = 0; f < 6; f++)
	{
		outportw(cdromdevice.command + 0, mdx[f]);
	}
	getIDEError(cdromdevice);
	waitForIDEFire();
	ide_wait_for_ready(cdromdevice);
	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);
}

void atapi_read_sector(IDEDevice cdromdevice, unsigned long lba, unsigned char count, unsigned short *location)
{

	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);

	outportb(cdromdevice.command + 6, cdromdevice.slave == 1 ? 0xB0 : 0xA0);
	outportb(cdromdevice.command + 1, 0x00);
	outportb(cdromdevice.command + 4, ATAPI_SECTOR_SIZE & 0xff);
	outportb(cdromdevice.command + 5, ATAPI_SECTOR_SIZE >> 8);
	outportb(cdromdevice.command + 7, 0xA0);

	getIDEError(cdromdevice);

	ide_wait_for_ready(cdromdevice);

	read_cmd[9] = count;
	read_cmd[2] = (lba >> 0x18) & 0xFF; /* most sig. byte of LBA */
	read_cmd[3] = (lba >> 0x10) & 0xFF;
	read_cmd[4] = (lba >> 0x08) & 0xFF;
	read_cmd[5] = (lba >> 0x00) & 0xFF;
	read_cmd[0] = 0xA8;

	resetIDEFire();
	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);
	unsigned short *mdx = (unsigned short *)&read_cmd;
	ide_wait_for_ready(cdromdevice);
	for (int f = 0; f < 6; f++)
	{
		outportw(cdromdevice.command + 0, mdx[f]);
	}
	getIDEError(cdromdevice);
	waitForIDEFire();
	ide_wait_for_ready(cdromdevice);
	unsigned short size = (((int)inportb(cdromdevice.command + 5)) << 8) | (int)(inportb(cdromdevice.command + 4));
	ide_wait_for_ready(cdromdevice);
	int mp = 0;
	for (unsigned short i = 0; i < (size / 2); i++)
	{
		if (getIDEError(cdromdevice) == 1)
		{
			return;
		}
		ide_wait_for_ready(cdromdevice);
		location[mp++] = inportw(cdromdevice.command + 0);
	}
	ide_wait_for_ready(cdromdevice);
}

void atapi_read_raw(Device *dev, unsigned long lba, unsigned char count, unsigned short *location)
{
	IDEDevice ide;
	ide.command = dev->arg1;
	ide.control = dev->arg2;
	ide.irq = dev->arg3;
	ide.slave = dev->arg4;
	atapi_read_sector(ide, lba, count, location);
}

void atapi_write_sector(IDEDevice cdromdevice, unsigned long lba, unsigned char count, unsigned short *location)
{
	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);

	outportb(cdromdevice.command + 6, cdromdevice.slave == 1 ? 0xB0 : 0xA0);
	outportb(cdromdevice.command + 1, 0x00);
	outportb(cdromdevice.command + 4, ATAPI_SECTOR_SIZE & 0xff);
	outportb(cdromdevice.command + 5, ATAPI_SECTOR_SIZE >> 8);
	outportb(cdromdevice.command + 7, 0xA0);

	getIDEError(cdromdevice);

	ide_wait_for_ready(cdromdevice);

	read_cmd[9] = count;
	read_cmd[2] = (lba >> 0x18) & 0xFF; /* most sig. byte of LBA */
	read_cmd[3] = (lba >> 0x10) & 0xFF;
	read_cmd[4] = (lba >> 0x08) & 0xFF;
	read_cmd[5] = (lba >> 0x00) & 0xFF;
	read_cmd[0] = 0xAA;

	resetIDEFire();
	getIDEError(cdromdevice);
	ide_wait_for_ready(cdromdevice);
	unsigned short *mdx = (unsigned short *)&read_cmd;
	ide_wait_for_ready(cdromdevice);
	for (int f = 0; f < 6; f++)
	{
		outportw(cdromdevice.command + 0, mdx[f]);
	}
	getIDEError(cdromdevice);
	waitForIDEFire();
	ide_wait_for_ready(cdromdevice);
	unsigned short size = (((int)inportb(cdromdevice.command + 5)) << 8) | (int)(inportb(cdromdevice.command + 4));
	ide_wait_for_ready(cdromdevice);
	int mp = 0;
	for (unsigned short i = 0; i < (size / 2); i++)
	{
		if (getIDEError(cdromdevice) == 1)
		{
			return;
		}
		ide_wait_for_ready(cdromdevice);
		outportw(cdromdevice.command + 0,location[mp++]);
	}
	ide_wait_for_ready(cdromdevice);
}

void atapi_write_raw(Device *dev, unsigned long lba, unsigned char count, unsigned short *location)
{
	IDEDevice ide;
	ide.command = dev->arg1;
	ide.control = dev->arg2;
	ide.irq = dev->arg3;
	ide.slave = dev->arg4;
	atapi_write_sector(ide, lba, count, location);
}

void ata_read_sector(IDEDevice hdddevice, unsigned long LBA, unsigned char count, unsigned short *location)
{
	ide_wait_for_ready(hdddevice);
	unsigned char cunt = count;
	resetIDEFire();
	outportb(hdddevice.command + 6, 0xE0 | (hdddevice.slave << 4) | ((LBA >> 24) & 0x0F));
	outportb(hdddevice.command + 2, (unsigned char)cunt);
	outportb(hdddevice.command + 3, (unsigned char)LBA & 0xFF);
	outportb(hdddevice.command + 4, (unsigned char)(LBA >> 8) & 0xFF);
	outportb(hdddevice.command + 5, (unsigned char)(LBA >> 16) & 0xFF);
	outportb(hdddevice.command + 7, 0x20);
	//getIDEError(hdddevice);
	ide_wait_for_ready(hdddevice);
	getIDEError(hdddevice);
	int U = 0;
	int i = 0;
	unsigned char *buffer = (unsigned char *) location;
	for (i = 0; i < 256; i++)
	{
		unsigned short tA = inportw(hdddevice.command);
		buffer[U++] = tA & 0xFF;
		buffer[U++] = (tA>>8) & 0xFF;
	}
}

void ata_write_sector(IDEDevice hdddevice, unsigned long LBA, unsigned char count, unsigned short *location)
{
	unsigned char cunt = count;
	resetIDEFire();
	outportb(hdddevice.command + 6, 0xE0 | (hdddevice.slave << 4) | ((LBA >> 24) & 0x0F));
	outportb(hdddevice.command + 2, (unsigned char)cunt);
	outportb(hdddevice.command + 3, (unsigned char)LBA);
	outportb(hdddevice.command + 4, (unsigned char)(LBA >> 8));
	outportb(hdddevice.command + 5, (unsigned char)(LBA >> 16));
	outportb(hdddevice.command + 7, 0x30);
	getIDEError(hdddevice);
	ide_wait_for_ready(hdddevice);
	int U = 0;
	int i = 0;
	for (i = 0; i < ((512*cunt) / 2); i++)
	{
		outportw(hdddevice.command, location[U++]);
		sleep(1);
	}
	resetIDEFire();
	outportb(hdddevice.command + 6, 0xE0 | (hdddevice.slave << 4));
	outportb(hdddevice.command + 2, (unsigned char)0);
	outportb(hdddevice.command + 3, (unsigned char)0);
	outportb(hdddevice.command + 4, (unsigned char)0);
	outportb(hdddevice.command + 5, (unsigned char)0);
	outportb(hdddevice.command + 7, 0xE7);
	getIDEError(hdddevice);
	ide_wait_for_ready(hdddevice);
}

void ata_write_raw(Device *dxv, unsigned long LBA, unsigned char count, unsigned short *location)
{
	unsigned char tok = count;
	IDEDevice *dev = (IDEDevice *)dxv->arg1;
	IDEDevice device;
	device.command = dev->command;
	device.control = dev->control;
	device.irq = dev->irq;
	device.slave = dev->slave;
	ata_write_sector(device, dxv->arg2 + LBA, tok, location);
}

void ata_read_raw(Device *dxv, unsigned long LBA, unsigned char count, unsigned short *location)
{
	unsigned char tok = count;
	IDEDevice *dev = (IDEDevice *)dxv->arg1;
	IDEDevice device;
	device.command = dev->command;
	device.control = dev->control;
	device.irq = dev->irq;
	device.slave = dev->slave;
	ata_read_sector(device, dxv->arg2 + LBA, tok, location);
}

char issata = 0;

void init_ide_device(IDEDevice device)
{
	setNormalInt(device.irq, (unsigned long)ideirq);
	printstring("IDE: initialising device CMD=");
	hexdump(device.command);
	printstring(" CTRL=");
	hexdump(device.control);
	printstring(" IRQ=");
	hexdump(device.irq);
	printstring(" SLV=");
	printstring(device.slave == 1 ? "SLAVE" : "MASTER");
	printstring("\n");

	// first, softreset
	outportb(device.control, 0x04);
	outportb(device.control, 0x00);

	outportb(device.command + 0x06, 0xA0 | device.slave << 4);

	resetTicks();
	while (1)
	{
		if ((inportb(device.command + 7) & 0x80) > 0)
		{
			break;
		}
		else if (!(inportb(device.command + 4) == 0 && inportb(device.command + 5) == 0))
		{
			break;
		}
		else if (getTicks() == 2)
		{
			break;
		}
	}

	unsigned char cl = inportb(device.command + 0x04);
	unsigned char ch = inportb(device.command + 0x05);

	if (cl == 0xFF && ch == 0xFF)
	{
		printf("[IDE] return\n");
		return;
	}

	outportb(device.command + 0x01, 1);
	outportb(device.control, 0);
	outportb(device.command + 0x06, 0xA0 | device.slave << 4);
	resetTicks();
	while (1)
	{
		if ((inportb(device.command + 7) & 0x80) > 0)
		{
			break;
		}
		else if (!(inportb(device.command + 4) == 0 && inportb(device.command + 5) == 0))
		{
			break;
		}
		else if (getTicks() == 2)
		{
			break;
		}
	}
	outportb(device.command + 0x07, 0xEC);
	resetTicks();
	while (1)
	{
		if ((inportb(device.command + 7) & 0x80) > 0)
		{
			break;
		}
		else if (!(inportb(device.command + 4) == 0 && inportb(device.command + 5) == 0))
		{
			break;
		}
		else if (getTicks() == 2)
		{
			break;
		}
	}
	cl = inportb(device.command + 0x04);
	ch = inportb(device.command + 0x05);
	if (!(cl == 0x14 && ch == 0xEB) && !(cl == 0x69 && ch == 0x96))
	{
		// maybe hdd?
		printf("[IDE] maybe ata\n");
		// ATA device detected!
		unsigned char *buffer = (unsigned char *)0x1000;
		ata_read_sector(device, 0, 1, (unsigned short *)buffer);
		ata_read_sector(device, 0, 1, (unsigned short *)buffer);
		if (buffer[510] == 0x55 && buffer[511] == 0xAA)
		{
			printf("[ATA] hdd is bootable!\n");
		}
		else
		{
			printf("[ATA] hdd is not bootable! (%x %x)\n",buffer[510],buffer[511]);
			if(buffer[510]==0xFF&&buffer[511]==0xFF)
			{
				return;
			}
			else
			{
				//for(int i = 0 ; i < 512 ; i++){printf("%x ",buffer[i]);}for(;;);
				return;
			}
		}

		Device *regdev = (Device *)malloc(sizeof(Device));

		regdev->readRawSector = (unsigned long)&ata_read_raw;
		regdev->writeRawSector = (unsigned long)&ata_write_raw;

		regdev->arg1 = (unsigned long)&device;
		regdev->arg2 = 0;
		regdev->arg3 = 0;
		regdev->arg4 = 0;
		regdev->arg5 = 512;
		detectFilesystemsOnMBR(regdev);
	}
	else
	{
		// is cdrom
		printf("[IDE] maybe atapi\n");
		outportb(device.command + 6, device.slave == 1 ? 0xB0 : 0xA0);
		outportb(device.command + 2, 0);
		outportb(device.command + 3, 0);
		outportb(device.command + 4, 0);
		outportb(device.command + 5, 0);
		outportb(device.command + 7, 0xA1);

		resetTicks();
		while (1)
		{
			if (getTicks() == 1)
			{
				break;
			}
		}

		if (inportb(device.command + 7) == 0)
		{
			printstring("IDE: device does not exist!\n");
			return;
		}
		if (getIDEError(device) == 0)
		{
			printstring("IDE: device is ATAPI\n");
			for (int i = 0; i < 256; i++)
			{
				inportw(device.command);
			}
			unsigned char *buffer = (unsigned char *)0x2000;
			atapi_read_sector(device, 0, 1, (unsigned short *)buffer);
			if (buffer[510] == 0x55 && buffer[511] == 0xAA)
			{
				printf("ATAPI: cdrom is bootable!\n");
			}
			else
			{
				printf("ATAPI: cdrom is not bootable!\n");
				return;
			}
			int choice = -1;
			for (int i = 0; i < 10; i++)
			{
				atapi_read_sector(device, 0x10 + i, 1, (unsigned short *)buffer);
				if (buffer[1] == 'C' && buffer[2] == 'D' && buffer[3] == '0' && buffer[4] == '0' && buffer[5] == '1')
				{
					choice = i;
					break;
				}
			}
			if (choice == -1)
			{
				printf("ATAPI: unknown filesystem\n");
			}
			else
			{
				printf("ATAPI: known filesystem ISO 9660\n");

				Device *regdev = getNextFreeDevice();

				regdev->readRawSector = (unsigned long)&atapi_read_raw;
				regdev->writeRawSector = (unsigned long)&atapi_write_raw;
				//				regdev->reinitialise 	= (unsigned long)&atapi_reset_raw;
				regdev->eject = (unsigned long)&atapi_eject_raw;

				regdev->dir = (unsigned long)&iso_9660_dir;
				regdev->readFile = (unsigned long)&iso_9660_read;
				regdev->existsFile = (unsigned long)&iso_9660_exists;

				// .command= 0x1f0,.control=0x3f6,.irq=14,.slave=0
				regdev->arg1 = device.command;
				regdev->arg2 = device.control;
				regdev->arg3 = device.irq;
				regdev->arg4 = device.slave;
				regdev->arg5 = ATAPI_SECTOR_SIZE;
			}
		}
	}
}

void init_ide2()
{
	init_ide_device(ata1);
	if (issata == 0)
	{
		init_ide_device(ata2);
		init_ide_device(ata3);
		init_ide_device(ata4);
	}
}

void init_ide(unsigned short BAR)
{
	printstring("IDE: initialisation started!\n");
	if (BAR & 0x01)
	{
		BAR--;
		printstring("IDE: IDE attached to IO port at base ");
		hexdump(BAR);
		printstring("\n");
	}
	else
	{
		printstring("IDE: IDE attached to memorylocation ");
		hexdump(BAR);
		printstring("\n");
		printstring("IDE: memory location not supported!!\n");
		return;
	}

	unsigned long atamode = inportl(BAR);
	if (atamode & 1)
	{
		printstring("IDE: IDE controller in native mode\n");
	}
	else
	{
		printstring("IDE: IDE controller in compatebility mode\n");
		printstring("IDE: Compatebility mode not supported\n");
		//		return;
	}

	init_ide_device(ata1);
	if (issata == 0)
	{
		init_ide_device(ata2);
		init_ide_device(ata3);
		init_ide_device(ata4);
	}
}
