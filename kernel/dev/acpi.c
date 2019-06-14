#include "../kernel.h"

long *SMI_CMD;
char ACPI_ENABLE;
char ACPI_DISABLE;
long *PM1a_CNT;
long *PM1b_CNT;
short SLP_TYPa;
short SLP_TYPb;
short SLP_EN;
short SCI_EN;
char PM1_CNT_LEN;



struct RSDPtr{
   char Signature[8];
   char CheckSum;
   char OemID[6];
   char Revision;
   long *RsdtAddress;
};



struct FACP{
   char Signature[4];
   long Length;
   char unneded1[40 - 8];
   long *DSDT;
   char unneded2[48 - 44];
   long *SMI_CMD;
   char ACPI_ENABLE;
   char ACPI_DISABLE;
   char unneded3[64 - 54];
   long *PM1a_CNT_BLK;
   long *PM1b_CNT_BLK;
   char unneded4[89 - 72];
   char PM1_CNT_LEN;
};

int acpiCheckHeader(unsigned int *ptr, char *sig){
	if (memcmp((void *)ptr, sig, 4) == 0){
		char *checkPtr = (char *) ptr;
		int len = *(ptr + 1);
		char check = 0;
		while (0<len--){
         		check += *checkPtr;
         		checkPtr++;
      		}
      		if (check == 0)
         		return 0;
   	}
   	return -1;
}

unsigned int acpiCheckRSDPtr(unsigned int *ptr){
	char *sig = "RSD PTR ";
	struct RSDPtr *rsdp = (struct RSDPtr *) ptr;
	char *bptr;
	char check = 0;
	unsigned int i;

	if (memcmp(sig, (void *)rsdp, 8) == 0){
		// check checksum rsdpd
		bptr = (char *) ptr;
		for (i=0; i<sizeof(struct RSDPtr); i++){
			check += *bptr;
			bptr++;
		}
		// found valid rsdpd   
		if (check == 0) {
         		return (unsigned int) rsdp->RsdtAddress;
      		}
	}
	return 0;
}

void init_acpi(){
	unsigned int *addr;
	unsigned int *rsdp;
	for (addr = (unsigned int *) 0x000E0000; (int) addr<0x00100000; addr += 0x10/sizeof(addr)){
		rsdp = (unsigned int *)acpiCheckRSDPtr(addr);
		if(rsdp!=0){
			goto success;
		}
	}
	
	int ebda = *((short *) 0x40E);
	ebda = ebda*0x10 &0x000FFFFF;
	for (addr = (unsigned int *) ebda; (int) addr<ebda+1024; addr+= 0x10/sizeof(addr)){
		rsdp = (unsigned int *)acpiCheckRSDPtr(addr);
		if(rsdp!=0){
			goto success;
		}
	}
	return;
	success:
	printf("ACPI: ACPI pressent at %x \n",rsdp);
	if(acpiCheckHeader(rsdp, "RSDT") == 0){
		// the RSDT contains an unknown number of pointers to acpi tables
      		int entrys = *(rsdp + 1);
      		entrys = (entrys-36) /4;
      		rsdp += 36/4;   // skip header information

      		while (0<entrys--){
         		// check if the desired table is reached
         		if (acpiCheckHeader((unsigned int *) *rsdp, "FACP") == 0){
            			entrys = -2;
            			struct FACP *facp = (struct FACP *) *rsdp;
            			if (acpiCheckHeader((unsigned int *) facp->DSDT, "DSDT") == 0){
               				// search the \_S5 package in the DSDT
               				char *S5Addr = (char *) facp->DSDT +36; // skip header
               				int dsdtLength = *(facp->DSDT+1) -36;
               				while (0 < dsdtLength--){
                  				if ( memcmp(S5Addr, "_S5_", 4) == 0)
                     					break;
                  				S5Addr++;
               				}
               				// check if \_S5 was found
               				if (dsdtLength > 0){
                  				// check for valid AML structure
                  				if ( ( *(S5Addr-1) == 0x08 || ( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') ) && *(S5Addr+4) == 0x12 ){
                     					S5Addr += 5;
                     					S5Addr += ((*S5Addr &0xC0)>>6) +2;   // calculate PkgLength size

                     					if (*S5Addr == 0x0A)
                        					S5Addr++;   // skip byteprefix
                     					SLP_TYPa = *(S5Addr)<<10;
                     					S5Addr++;

                     					if (*S5Addr == 0x0A)
                        					S5Addr++;   // skip byteprefix
                     					SLP_TYPb = *(S5Addr)<<10;

                     					SMI_CMD = facp->SMI_CMD;

                     					ACPI_ENABLE = facp->ACPI_ENABLE;
                     					ACPI_DISABLE = facp->ACPI_DISABLE;

                     					PM1a_CNT = facp->PM1a_CNT_BLK;
                     					PM1b_CNT = facp->PM1b_CNT_BLK;
                     
                     					PM1_CNT_LEN = facp->PM1_CNT_LEN;

                     					SLP_EN = 1<<13;
                     					SCI_EN = 1;
							
							printf("ACPI: everything is ready!\n");
                     					return;
                  				} else {
                     					printf("ACPI: \\_S5 parse error.\n");
                  				}
               				} else {
                  				printf("ACPI: \\_S5 not present.\n");
               				}
            			} else {
               				printf("ACPI: DSDT invalid.\n");
            			}
         		}
         		rsdp++;
      		}
      		printf("ACPI: no valid FACP present.\n");
	}
	for(;;);
}

int acpiEnable(void){
	// check if acpi is enabled
	if ( (inportw((unsigned int) PM1a_CNT) &SCI_EN) == 0 ){
		// check if acpi can be enabled
		if (SMI_CMD != 0 && ACPI_ENABLE != 0){
			outportb((unsigned int) SMI_CMD, ACPI_ENABLE); // send acpi enable command
			// give 3 seconds time to enable acpi
			int i;
			for (i=0; i<300; i++ ){
				if ( (inportw((unsigned int) PM1a_CNT) &SCI_EN) == 1 )
					break;
				sleep(10);
			}
			if (PM1b_CNT != 0)
				for (; i<300; i++ ){
					if ( (inportw((unsigned int) PM1b_CNT) &SCI_EN) == 1 )
						break;
					sleep(10);
				}
			if (i<300) {
				printf("ACPI: enabled acpi.\n");
				return 0;
			} else {
				printf("ACPI: couldn't enable acpi.\n");
            			return -1;
         		}
      		} else {
         		printf("ACPI: no known way to enable acpi.\n");
         		return -1;
      		}
   	} else {
      		printf("ACPI: acpi was already enabled.\n");
      		return 0;
   	}
}


void poweroff(){
	if (SCI_EN == 0)
      		return;
	if(acpiEnable()==0){
		outportw((unsigned int) PM1a_CNT, SLP_TYPa | SLP_EN );
		if ( PM1b_CNT != 0 )
      			outportw((unsigned int) PM1b_CNT, SLP_TYPb | SLP_EN );
      		printf("ACPI: Panic! unable to turn off");
      		for(;;);
	}else{
		printf("ACPI: Unable to enable ACPI!\n");
		for(;;);
	}
}
