#include "../kernel.h"

unsigned char *ipaddr;
unsigned char *macaddr;

void tftp_read(Device *device,char* path,char *buffer){
    int pathlength = strlen(path);
    char* type = "netascii";
    int typelength = strlen(type);
    int packagelength = sizeof(struct UDPHeader) + pathlength + 4 + typelength;
    unsigned char* package = (unsigned char*) malloc(packagelength);
    struct UDPHeader *rpackage = (struct UDPHeader*) package;
    debugf("[TFTP] TFTP server %d.%d.%d.%d, reading file %s \n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3],path);
    fillUdpHeader(rpackage,macaddr,packagelength-sizeof(struct EthernetHeader),getOurIpAsLong(),((unsigned long*)ipaddr)[0],50618,69);
    // opcode
    package[sizeof(struct UDPHeader)+0] = 0;
    package[sizeof(struct UDPHeader)+1] = 1;
    for(int i = 0 ; i < pathlength ; i++){
        package[sizeof(struct UDPHeader)+2+i] = path[i];
    }
    package[sizeof(struct UDPHeader)+2+pathlength] = 0;
    for(int i = 0 ; i < typelength ; i++){
        package[sizeof(struct UDPHeader)+2+pathlength+1+i] = type[i];
    }
    package[sizeof(struct UDPHeader)+2+pathlength+1+typelength] = 0;

    PackageRecievedDescriptor sec;
    sec.buffersize = packagelength;
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)package;

    sendEthernetPackage(sec);
    int memcopoffset = 0;
    PackageRecievedDescriptor prd;
    struct EthernetHeader *eh;
    int indexid = 1;
    again:
    while(1){
        prd = getEthernetPackage();
        eh = (struct EthernetHeader*) prd.low_buf;
        if(eh->type==ETHERNET_TYPE_IP4){
            break;
        }
    }
    unsigned char* tow = (unsigned char*) prd.low_buf;
    if(tow[sizeof(struct UDPHeader)+1]!=3){
        debugf("[TFTP] Unexpected instruction: %x [%x] !\n",tow[sizeof(struct UDPHeader)+1],tow[sizeof(struct UDPHeader)]);
        goto again;
    }
    unsigned short indexnow = tow[sizeof(struct UDPHeader)+3];
    indexnow += tow[sizeof(struct UDPHeader)+2]*0x100;
    if(indexid!=indexnow){
        debugf("[TFTP] Out of sync. exp: %x found: %x !\n",indexid,indexnow);
        goto again;
    }
    int over = prd.buffersize-sizeof(struct UDPHeader)-4;
    for(int i = 0 ; i < over ; i++){
        buffer[memcopoffset++] = tow[sizeof(struct UDPHeader)+4+i];
    }
    packagelength = sizeof(struct UDPHeader) + 4 ;
    package = (unsigned char*) malloc(packagelength);
    rpackage = (struct UDPHeader*) package;
    fillUdpHeader(rpackage,macaddr,packagelength-sizeof(struct EthernetHeader),getOurIpAsLong(),((unsigned long*)ipaddr)[0],50618,69);
    package[sizeof(struct UDPHeader)+0] = 0;
    package[sizeof(struct UDPHeader)+1] = 4;
    package[sizeof(struct UDPHeader)+2] = (indexid/0x100) & 0xFF;
    package[sizeof(struct UDPHeader)+3] = indexid & 0xFF;

    sec.buffersize = packagelength;
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)package;

    sendEthernetPackage(sec);

    if(over==512){
        goto again;
    }
    buffer[memcopoffset++] = 0;
    debugf("[TFTP] endofcopy\n");
}

void tftp_dir(Device *device,char* path,char *buffer){
    tftp_read(device,"dir.txt",buffer);
}

char tftp_exists(Device *device,char* path){
    return 1;
}

void initialiseTFTP(Device *device){
    // arg4 = IP
    // arg5 = MAC
    ipaddr = (unsigned char*) memdup(device->arg4,4);
    macaddr = (unsigned char*) memdup(device->arg5,6);
    debugf("[TFTP] TFTP Module loaded for IP %d.%d.%d.%d and MAC %x:%x:%x:%x:%x:%x \n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3],macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);

	device->dir = (unsigned long)&tftp_dir;
	device->readFile = (unsigned long)&tftp_read;
	device->existsFile = (unsigned long)&tftp_exists;
}