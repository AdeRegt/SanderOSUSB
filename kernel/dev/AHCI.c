#include "../kernel.h"

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier
 
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4
 
#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define HBA_PxIS_TFES   (1 << 30)
#define ATA_CMD_READ_DMA_EXT 0x25

// https://wiki.osdev.org/AHCI 

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

typedef struct tagFIS_REG_H2D
{
	// DWORD 0
	unsigned char  fis_type;	// FIS_TYPE_REG_H2D
 
	unsigned char  pmport:4;	// Port multiplier
	unsigned char  rsv0:3;		// Reserved
	unsigned char  c:1;		// 1: Command, 0: Control
 
	unsigned char  command;	// Command register
	unsigned char  featurel;	// Feature register, 7:0
 
	// DWORD 1
	unsigned char  lba0;		// LBA low register, 7:0
	unsigned char  lba1;		// LBA mid register, 15:8
	unsigned char  lba2;		// LBA high register, 23:16
	unsigned char  device;		// Device register
 
	// DWORD 2
	unsigned char  lba3;		// LBA register, 31:24
	unsigned char  lba4;		// LBA register, 39:32
	unsigned char  lba5;		// LBA register, 47:40
	unsigned char  featureh;	// Feature register, 15:8
 
	// DWORD 3
	unsigned char  countl;		// Count register, 7:0
	unsigned char  counth;		// Count register, 15:8
	unsigned char  icc;		// Isochronous command completion
	unsigned char  control;	// Control register
 
	// DWORD 4
	unsigned char  rsv1[4];	// Reserved
} FIS_REG_H2D;

typedef struct tagFIS_REG_D2H
{
	// DWORD 0
	unsigned char  fis_type;    // FIS_TYPE_REG_D2H
 
	unsigned char  pmport:4;    // Port multiplier
	unsigned char  rsv0:2;      // Reserved
	unsigned char  i:1;         // Interrupt bit
	unsigned char  rsv1:1;      // Reserved
 
	unsigned char  status;      // Status register
	unsigned char  error;       // Error register
 
	// DWORD 1
	unsigned char  lba0;        // LBA low register, 7:0
	unsigned char  lba1;        // LBA mid register, 15:8
	unsigned char  lba2;        // LBA high register, 23:16
	unsigned char  device;      // Device register
 
	// DWORD 2
	unsigned char  lba3;        // LBA register, 31:24
	unsigned char  lba4;        // LBA register, 39:32
	unsigned char  lba5;        // LBA register, 47:40
	unsigned char  rsv2;        // Reserved
 
	// DWORD 3
	unsigned char  countl;      // Count register, 7:0
	unsigned char  counth;      // Count register, 15:8
	unsigned char  rsv3[2];     // Reserved
 
	// DWORD 4
	unsigned char  rsv4[4];     // Reserved
} FIS_REG_D2H;

typedef struct tagFIS_DATA
{
	// DWORD 0
	unsigned char  fis_type;	// FIS_TYPE_DATA
 
	unsigned char  pmport:4;	// Port multiplier
	unsigned char  rsv0:4;		// Reserved
 
	unsigned char  rsv1[2];	// Reserved
 
	// DWORD 1 ~ N
	unsigned long data[1];	// Payload
} FIS_DATA;

typedef struct tagFIS_PIO_SETUP
{
	// DWORD 0
	unsigned char  fis_type;	// FIS_TYPE_PIO_SETUP
 
	unsigned char  pmport:4;	// Port multiplier
	unsigned char  rsv0:1;		// Reserved
	unsigned char  d:1;		// Data transfer direction, 1 - device to host
	unsigned char  i:1;		// Interrupt bit
	unsigned char  rsv1:1;
 
	unsigned char  status;		// Status register
	unsigned char  error;		// Error register
 
	// DWORD 1
	unsigned char  lba0;		// LBA low register, 7:0
	unsigned char  lba1;		// LBA mid register, 15:8
	unsigned char  lba2;		// LBA high register, 23:16
	unsigned char  device;		// Device register
 
	// DWORD 2
	unsigned char  lba3;		// LBA register, 31:24
	unsigned char  lba4;		// LBA register, 39:32
	unsigned char  lba5;		// LBA register, 47:40
	unsigned char  rsv2;		// Reserved
 
	// DWORD 3
	unsigned char  countl;		// Count register, 7:0
	unsigned char  counth;		// Count register, 15:8
	unsigned char  rsv3;		// Reserved
	unsigned char  e_status;	// New value of status register
 
	// DWORD 4
	unsigned short tc;		// Transfer count
	unsigned char  rsv4[2];	// Reserved
} FIS_PIO_SETUP;

typedef struct tagFIS_DMA_SETUP
{
	// DWORD 0
	unsigned char  fis_type;	// FIS_TYPE_DMA_SETUP
 
	unsigned char  pmport:4;	// Port multiplier
	unsigned char  rsv0:1;		// Reserved
	unsigned char  d:1;		// Data transfer direction, 1 - device to host
	unsigned char  i:1;		// Interrupt bit
	unsigned char  a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed
 
        unsigned char  rsved[2];       // Reserved
 
	//DWORD 1&2
 
        unsigned short DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
 	unsigned short DMAbufferID2;
        //DWORD 3
        unsigned long rsvd;           //More reserved
 
        //DWORD 4
        unsigned long DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0
 
        //DWORD 5
        unsigned long TransferCount;  //Number of bytes to transfer. Bit 0 must be 0
 
        //DWORD 6
        unsigned long resvd;          //Reserved
 
} FIS_DMA_SETUP;

typedef volatile struct tagHBA_FIS
{
	// 0x00
	FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
	unsigned char         pad0[4];
 
	// 0x20
	FIS_PIO_SETUP	psfis;		// PIO Setup FIS
	unsigned char         pad1[12];
 
	// 0x40
	FIS_REG_D2H	rfis;		// Register – Device to Host FIS
	unsigned char         pad2[4];
 
	// 0x58
	unsigned short	sdbfis;		// Set Device Bit FIS
 
	// 0x60
	unsigned char         ufis[64];
 
	// 0xA0
	unsigned char   	rsv[0x100-0xA0];
} HBA_FIS;

typedef struct tagHBA_CMD_HEADER
{
	// DW0
	unsigned char  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
	unsigned char  a:1;		// ATAPI
	unsigned char  w:1;		// Write, 1: H2D, 0: D2H
	unsigned char  p:1;		// Prefetchable
 
	unsigned char  r:1;		// Reset
	unsigned char  b:1;		// BIST
	unsigned char  c:1;		// Clear busy upon R_OK
	unsigned char  rsv0:1;		// Reserved
	unsigned char  pmp:4;		// Port multiplier port
 
	unsigned short prdtl;		// Physical region descriptor table length in entries
 
	// DW1
	volatile
	unsigned long prdbc;		// Physical region descriptor byte count transferred
 
	// DW2, 3
	unsigned long ctba;		// Command table descriptor base address
	unsigned long ctbau;		// Command table descriptor base address upper 32 bits
 
	// DW4 - 7
	unsigned long rsv1[4];	// Reserved
} HBA_CMD_HEADER;
 
typedef struct tagHBA_PRDT_ENTRY
{
	unsigned long dba;		// Data base address
	unsigned long dbau;		// Data base address upper 32 bits
	unsigned long rsv0;		// Reserved
 
	// DW3
	unsigned long dbc:22;		// Byte count, 4M max
	unsigned long rsv1:9;		// Reserved
	unsigned long i:1;		// Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL
{
	// 0x00
	unsigned char  cfis[64];	// Command FIS
 
	// 0x40
	unsigned char  acmd[16];	// ATAPI command, 12 or 16 bytes
 
	// 0x50
	unsigned char  rsv[48];	// Reserved
 
	// 0x80
	HBA_PRDT_ENTRY	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

typedef volatile struct tagHBA_PORT
{
	unsigned long clb;		// 0x00, command list base address, 1K-byte aligned
	unsigned long clbu;		// 0x04, command list base address upper 32 bits
	unsigned long fb;		// 0x08, FIS base address, 256-byte aligned
	unsigned long fbu;		// 0x0C, FIS base address upper 32 bits
	unsigned long is;		// 0x10, interrupt status
	unsigned long ie;		// 0x14, interrupt enable
	unsigned long cmd;		// 0x18, command and status
	unsigned long rsv0;		// 0x1C, Reserved
	unsigned long tfd;		// 0x20, task file data
	unsigned long sig;		// 0x24, signature
	unsigned long ssts;		// 0x28, SATA status (SCR0:SStatus)
	unsigned long sctl;		// 0x2C, SATA control (SCR2:SControl)
	unsigned long serr;		// 0x30, SATA error (SCR1:SError)
	unsigned long sact;		// 0x34, SATA active (SCR3:SActive)
	unsigned long ci;		// 0x38, command issue
	unsigned long sntf;		// 0x3C, SATA notification (SCR4:SNotification)
	unsigned long fbs;		// 0x40, FIS-based switch control
	unsigned long rsv1[11];	// 0x44 ~ 0x6F, Reserved
	unsigned long vendor[4];	// 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct tagHBA_MEM
{
	// 0x00 - 0x2B, Generic Host Control
	unsigned long cap;		// 0x00, Host capability
	unsigned long ghc;		// 0x04, Global host control
	unsigned long is;		// 0x08, Interrupt status
	unsigned long pi;		// 0x0C, Port implemented
	unsigned long vs;		// 0x10, Version
	unsigned long ccc_ctl;	// 0x14, Command completion coalescing control
	unsigned long ccc_pts;	// 0x18, Command completion coalescing ports
	unsigned long em_loc;		// 0x1C, Enclosure management location
	unsigned long em_ctl;		// 0x20, Enclosure management control
	unsigned long cap2;		// 0x24, Host capabilities extended
	unsigned long bohc;		// 0x28, BIOS/OS handoff control and status
 
	// 0x2C - 0x9F, Reserved
	unsigned char  rsv[0xA0-0x2C];
 
	// 0xA0 - 0xFF, Vendor specific registers
	unsigned char  vendor[0x100-0xA0];
 
	// 0x100 - 0x10FF, Port control registers
	HBA_PORT	ports[1];	// 1 ~ 32
} HBA_MEM;

#define	AHCI_BASE	0x400000	// 4M
 
#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000
#define ATA_CMD_READ_DMA_EX 0x25
 
 
// Start command engine
void start_cmd(HBA_PORT *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR)
		;
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stop_cmd(HBA_PORT *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
}

void port_rebase(HBA_PORT *port, int portno)
{
	stop_cmd(port);	// Stop command engine
 
	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);
 
	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);
 
	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}
 
	start_cmd(port);	// Start command engine
}

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
 
// Find a free command list slot
int find_cmdslot(HBA_PORT *port)
{
	// If not set in SACT and CI, the slot is free
	unsigned long slots = (port->sact | port->ci);
	for (int i=0; i<32; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	printf("Cannot find free command list entry\n");
	return -1;
}

int ahci_atapi_read(HBA_PORT *port, unsigned long startl, unsigned long starth, unsigned long count, unsigned short *buf)
{
	port->is = (unsigned long) -1;		// Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 0;
 
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(unsigned long);	// Command FIS size
	cmdheader->w = 0;		// Read from device
	cmdheader->prdtl = (unsigned short)((count-1)>>4) + 1;	// PRDT entries count
	cmdheader->a = 1;
 
	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
	// 8K bytes (16 sectors) per PRDT
	int i = 0;
//	for (i=0; i<cmdheader->prdtl-1; i++)
//	{
//		cmdtbl->prdt_entry[i].dba = (unsigned long) buf;
//		cmdtbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
//		cmdtbl->prdt_entry[i].i = 1;
//		buf += 4*1024;	// 4K words
//		count -= 16;	// 16 sectors
//	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (unsigned long) buf;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;
	
	unsigned long lba = startl;
    	cmdtbl->acmd[9] = count;
    	cmdtbl->acmd[2] = (lba >> 0x18) & 0xFF;   /* most sig. byte of LBA */
    	cmdtbl->acmd[3] = (lba >> 0x10) & 0xFF;
    	cmdtbl->acmd[4] = (lba >> 0x08) & 0xFF;
    	cmdtbl->acmd[5] = (lba >> 0x00) & 0xFF;
    	cmdtbl->acmd[0] = 0xA8;
 
	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
 
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = 0xA0;
 
	cmdfis->lba0 = (unsigned char)startl;
	cmdfis->lba1 = (unsigned char)(startl>>8);
	cmdfis->lba2 = (unsigned char)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode
 
	cmdfis->lba3 = (unsigned char)(startl>>24);
	cmdfis->lba4 = (unsigned char)starth;
	cmdfis->lba5 = (unsigned char)(starth>>8);
 
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		printf("Port is hung\n");for(;;);
		return 0;
	}
 
	port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			printf("Read disk error\n");for(;;);
			return 0;
		}
		
	}
 
	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		printf("Read disk error\n");for(;;);
		return 0;
	}
 
	return 1;
}

int ahci_atapi_eject(HBA_PORT *port)
{
	port->is = (unsigned long) -1;		// Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 0;
 
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(unsigned long);	// Command FIS size
	cmdheader->w = 0;		// Read from device
//	cmdheader->prdtl = (unsigned short)((count-1)>>4) + 1;	// PRDT entries count
	cmdheader->a = 1;
 
	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
	// 8K bytes (16 sectors) per PRDT
	int i = 0;
//	for (i=0; i<cmdheader->prdtl-1; i++)
//	{
//		cmdtbl->prdt_entry[i].dba = (unsigned long) buf;
//		cmdtbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
//		cmdtbl->prdt_entry[i].i = 1;
//		buf += 4*1024;	// 4K words
//		count -= 16;	// 16 sectors
//	}
	// Last entry
//	cmdtbl->prdt_entry[i].dba = (unsigned long) buf;
// 	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;
	
	cmdtbl->acmd[ 0] = 0x1B;
    	cmdtbl->acmd[ 1] = 0x00;
    	cmdtbl->acmd[ 2] = 0x00;
    	cmdtbl->acmd[ 3] = 0x00;
    	cmdtbl->acmd[ 4] = 0x02;
    	cmdtbl->acmd[ 5] = 0x00;
    	cmdtbl->acmd[ 6] = 0x00;
    	cmdtbl->acmd[ 7] = 0x00;
    	cmdtbl->acmd[ 8] = 0x00;
    	cmdtbl->acmd[ 9] = 0x00;
    	cmdtbl->acmd[10] = 0x00;
    	cmdtbl->acmd[11] = 0x00;
 
	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
 
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = 0xA0;
 
//	cmdfis->lba0 = (unsigned char)startl;
//	cmdfis->lba1 = (unsigned char)(startl>>8);
//	cmdfis->lba2 = (unsigned char)(startl>>16);
//	cmdfis->device = 1<<6;	// LBA mode
 //
//	cmdfis->lba3 = (unsigned char)(startl>>24);
//	cmdfis->lba4 = (unsigned char)starth;
//	cmdfis->lba5 = (unsigned char)(starth>>8);
 
//	cmdfis->countl = count & 0xFF;
//	cmdfis->counth = (count >> 8) & 0xFF;
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		printf("Port is hung\n");for(;;);
		return 0;
	}
 
	port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			printf("Read disk error\n");for(;;);
			return 0;
		}
		
	}
 
	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		printf("Read disk error\n");for(;;);
		return 0;
	}
 
	return 1;
}
 
int ahci_ata_read(HBA_PORT *port, unsigned long startl, unsigned long starth, unsigned long count, unsigned short *buf)
{
	port->is = (unsigned long) -1;		// Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return 0;
 
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(unsigned long);	// Command FIS size
	cmdheader->w = 0;		// Read from device
	cmdheader->prdtl = (unsigned short)((count-1)>>4) + 1;	// PRDT entries count
 
	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
	// 8K bytes (16 sectors) per PRDT
	int i = 0;
	for (i=0; i<cmdheader->prdtl-1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (unsigned long) buf;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (unsigned long) buf;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;
 
	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
 
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
 
	cmdfis->lba0 = (unsigned char)startl;
	cmdfis->lba1 = (unsigned char)(startl>>8);
	cmdfis->lba2 = (unsigned char)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode
 
	cmdfis->lba3 = (unsigned char)(startl>>24);
	cmdfis->lba4 = (unsigned char)starth;
	cmdfis->lba5 = (unsigned char)(starth>>8);
 
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		printf("Port is hung\n");for(;;);
		return 0;
	}
 
	port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			printf("Read disk error\n");for(;;);
			return 0;
		}
		
	}
 
	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		printf("Read disk error\n");for(;;);
		return 0;
	}
 
	return 1;
}

void ahci_init(int bus,int slot,int function){
	unsigned long bar0 = getBARaddress(bus,slot,function,0x10);
	unsigned long bar1 = getBARaddress(bus,slot,function,0x14);
	unsigned long bar2 = getBARaddress(bus,slot,function,0x18);
	unsigned long bar3 = getBARaddress(bus,slot,function,0x1C);
	unsigned long bar4 = getBARaddress(bus,slot,function,0x20);
	unsigned long bar5 = getBARaddress(bus,slot,function,0x24);
	unsigned long base = bar5;
	printf("[AHCI] AHCI detected based at %x !\n",base);
	printf("[AHCI] 0:%x  1:%x 2:%x 3:%x 4:%x 5:%x!\n",bar0,bar1,bar2,bar3,bar4,bar5);
	//init_ide2();
	HBA_MEM *target = (HBA_MEM *)base;
	unsigned short pi = target->pi;
	int i = 0;
	while (i<33){
		if (pi & 1){
			
			HBA_PORT *port = (HBA_PORT *)&target->ports[i];
			unsigned long ssts = port->ssts;
 
			unsigned char ipm = (ssts >> 8) & 0x0F;
			unsigned char det = ssts & 0x0F;
		 
			if (det != HBA_PORT_DET_PRESENT && ipm != HBA_PORT_IPM_ACTIVE){
				
		 	}else if(port->sig==SATA_SIG_ATAPI){
				printf("[AHCI] ATAPI detected\n");
				port_rebase(port,i);
				ahci_atapi_eject(port);
				unsigned char* msg = (unsigned char*) 0x1000;
				ahci_atapi_read(port, 0, 0, 1, (unsigned short *)msg);
				if(msg[510]==0x55&&msg[511]==0xAA){
					printf("[AHCI] ATAPI is bootable\n");
				}else{
					printf("[AHCI] ATAPI is not bootable\n");
				}
				printf("[AHCI] completed...\n");
			}else if(port->sig==SATA_SIG_SEMB){
				printf("[AHCI] SEMB detected\n");
			}else if(port->sig==SATA_SIG_PM){
				printf("[AHCI] PM detected\n");
			}else{
				printf("[AHCI] SATA detected %x \n",i);
				port_rebase(port,i);
				unsigned char* msg = (unsigned char*) 0x1000;
				ahci_ata_read(port, 0, 0, 1, (unsigned short *)msg);
				if(msg[510]==0x55&&msg[511]==0xAA){
					printf("[AHCI] ATA is bootable\n");
				}else{
					printf("[AHCI] ATA is not bootable\n");
				}
			}
		}
		pi >>= 1;
		i ++;
	}
	printf("[AHCI] End of operation\n");
}
