//
// includes.... everything in one file for simplicity
#include "../kernel.h"

unsigned short switch_endian16(unsigned short nb) {
    return (nb>>8) | (nb<<8);
}

unsigned long switch_endian32(unsigned long num) {
    return ((num>>24)&0xff) | // move byte 3 to byte 0
    ((num<<8)&0xff0000) | // move byte 1 to byte 2
    ((num>>8)&0xff00) | // move byte 2 to byte 1
    ((num<<24)&0xff000000); // byte 0 to byte 3
}

void ethernet_detect(int bus,int slot,int function,int device,int vendor){
    if((device==0x8168||device==0x8139)&&vendor==0x10ec){ 
        // Sander his RTL8169 driver comes here
        init_rtl(bus,slot,function);
    }else if(device==0x100e||device==0x153A||device==0x10EA||vendor==0x8086){
        // Johan his E1000 driver comes here
        init_e1000(bus,slot,function);
    }else{
        printf("[ETH] Unknown ethernet device: device: %x vendor: %x \n",device,vendor);
    }
}

EthernetDevice defaultEthernetDevice;
unsigned char our_ip[SIZE_OF_IP];
unsigned char router_ip[SIZE_OF_IP];
unsigned char dns_ip[SIZE_OF_IP];
unsigned char dhcp_ip[SIZE_OF_IP];

void ethernet_set_link_status(unsigned long a){
    defaultEthernetDevice.is_online = a;
}

void register_ethernet_device(unsigned long sendPackage,unsigned long recievePackage,unsigned char mac[8]){
    defaultEthernetDevice.recievePackage = recievePackage;
    defaultEthernetDevice.sendPackage = sendPackage;
    defaultEthernetDevice.is_enabled = 1;
    for(int i = 0 ; i < 8 ; i++){
        defaultEthernetDevice.mac[i] = mac[i];
    }
}

EthernetDevice getDefaultEthernetDevice(){
    return defaultEthernetDevice;
}

PackageRecievedDescriptor getEthernetPackage(){
    PackageRecievedDescriptor (*getPackage)() = (void*)defaultEthernetDevice.recievePackage;
    return getPackage();
}

int sendEthernetPackage(PackageRecievedDescriptor desc){
    int (*sendPackage)(PackageRecievedDescriptor desc) = (void*)defaultEthernetDevice.sendPackage;
    return sendPackage(desc);
}

void fillMac(unsigned char* to,unsigned char* from){
    for(int i = 0 ; i < SIZE_OF_MAC ; i++){
        to[i] = from[i];
    }
}

void fillIP(unsigned char* to,unsigned char* from){
    for(int i = 0 ; i < SIZE_OF_IP ; i++){
        to[i] = from[i];
    }
}

void fillEthernetHeader(struct EthernetHeader* eh, unsigned char* destip,unsigned short type){
    fillMac((unsigned char*)&eh->to,destip);
    fillMac((unsigned char*)&eh->from,(unsigned char*)&defaultEthernetDevice.mac);
    eh->type = type;
}

unsigned char* getMACFromIp(unsigned char* ip){
    struct ARPHeader* arpie = (struct ARPHeader*)malloc(sizeof(struct ARPHeader));
    unsigned char everyone[SIZE_OF_MAC] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    unsigned char empty[SIZE_OF_MAC] = {0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char prefip[SIZE_OF_IP] = {192,168,5,5};
    fillEthernetHeader((struct EthernetHeader*) &arpie->ethernetheader,everyone,ETHERNET_TYPE_ARP);
    arpie->hardware_type = 0x0100;
    arpie->protocol_type = 0x0008;
    arpie->hardware_address_length = SIZE_OF_MAC;
    arpie->protocol_address_length = SIZE_OF_IP;
    arpie->operation = 0x0100;

    fillMac((unsigned char*)&arpie->source_mac,(unsigned char*)&defaultEthernetDevice.mac);
    fillIP((unsigned char*)&arpie->source_ip,(unsigned char*)&prefip);

    fillMac((unsigned char*)&arpie->dest_mac,(unsigned char*)&empty);
    fillIP((unsigned char*)&arpie->dest_ip,ip);
    
    PackageRecievedDescriptor sec;
    sec.buffersize = sizeof(struct ARPHeader);
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)arpie;

    sendEthernetPackage(sec);
    struct ARPHeader* ah;
    while(1){
        PackageRecievedDescriptor prd = getEthernetPackage();
        struct EthernetHeader *eh = (struct EthernetHeader*) prd.low_buf;
        if(eh->type==ETHERNET_TYPE_ARP){
            ah = (struct ARPHeader*) prd.low_buf;
            if(ah->operation==0x0200){
                break;
            }
        }
    }
    debugf("[ETH] %d.%d.%d.%d is at %x:%x:%x:%x:%x:%x \n",ip[0],ip[1],ip[2],ip[3],ah->source_mac[0],ah->source_mac[1],ah->source_mac[2],ah->source_mac[3],ah->source_mac[4],ah->source_mac[5]);
    return ah->source_mac;
}

unsigned char* getOurRouterIp(){
    return (unsigned char*) &router_ip;
}

unsigned short ipv4count = 1;
void fillIpv4Header(struct IPv4Header *ipv4header, unsigned char* destmac, unsigned short length,unsigned char protocol,unsigned long from, unsigned long to){
    fillEthernetHeader((struct EthernetHeader*)&ipv4header->ethernetheader,destmac,ETHERNET_TYPE_IP4);
    ipv4header->version = 4;
    ipv4header->internet_header_length = 5;
    ipv4header->type_of_service = 0;
    ipv4header->total_length = switch_endian16( length );
    ipv4header->id = switch_endian16(ipv4count);
    ipv4header->flags = 0;
    ipv4header->fragment_offset= 0b01000;
    ipv4header->time_to_live = 64;
    ipv4header->protocol = protocol;
    ipv4header->checksum = 0;
    ipv4header->source_addr = from;
    ipv4header->dest_addr = to;

    unsigned long checksum = 0;
    checksum += 0x4500;
    checksum += length;
    checksum += ipv4count++;
    checksum += 0x4000;
    checksum += 0x4000 + protocol;
    checksum += switch_endian16((from >> 16) & 0xFFFF);
    checksum += switch_endian16(from & 0xFFFF); 
    checksum += switch_endian16((to >> 16) & 0xFFFF);
    checksum += switch_endian16(to & 0xFFFF);
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    ipv4header->checksum = switch_endian16((unsigned short) (~checksum));
}

void fillUdpHeader(struct UDPHeader *udpheader, unsigned char *destmac, unsigned short size,unsigned long from, unsigned long to,unsigned short source_port, unsigned short destination_port){
    fillIpv4Header((struct IPv4Header*)&udpheader->ipv4header,destmac,size,IPV4_TYPE_UDP,from,to);

    udpheader->length = switch_endian16(size - (sizeof(struct IPv4Header)-sizeof(struct EthernetHeader)));
    udpheader->destination_port = switch_endian16(destination_port);
    udpheader->source_port = switch_endian16(source_port);

    udpheader->checksum = 0;
}

// checksum for tcp by https://github.com/nelsoncole/sirius-x86-64/blob/main/kernel/driver/net/checksum.c 
struct tcp_checksum_header{
    unsigned int src;
    unsigned int dst;
    unsigned char rsved;
    unsigned char protocol;
    unsigned short len;
    unsigned short source_port;
    unsigned short destination_port;
    unsigned long sequence_number;
    unsigned long acknowledge_number;
    unsigned short flags;
    unsigned short window_size;
    unsigned short checksum;
    unsigned short urgent_pointer;
}__attribute__((packed));

unsigned short net_checksum(const unsigned char *start, const unsigned char *end)
{

    unsigned int checksum = 0;
    unsigned int len = end - start;
    unsigned short *p = (unsigned short *)start;

    // acc
    while (len > 1) {
        checksum += *p++;
        len -= 2;
    }

    if (len != 0) {
        checksum += *(unsigned char *)p;
    }


    checksum = (checksum & 0xffff) + (checksum >> 16);
    checksum += (checksum >> 16);

    unsigned short final = ~checksum;

    return switch_endian16(final);
}

unsigned long taaaX = 0;
unsigned long taaaY = 0;
void fillTcpHeader(struct TCPHeader *tcpheader,unsigned char *destmac,unsigned short size,unsigned long from,unsigned long to,unsigned short from_port,unsigned short to_port,unsigned long sequence_number,unsigned long acknowledge_number,unsigned char header_length,unsigned short flags,unsigned short window){
    fillIpv4Header((struct IPv4Header*)&tcpheader->header,destmac,size,IPV4_TYPE_TCP,from,to);
    if(sequence_number==0x1010&&acknowledge_number==0x1010){
        sequence_number = taaaX;
        acknowledge_number = taaaY;
        taaaX += (size + sizeof(struct EthernetHeader)) - sizeof(struct TCPHeader);
    }else if(flags&TCP_ACK){
        taaaX = sequence_number;
        taaaY = acknowledge_number;
    }
    tcpheader->source_port          = switch_endian16(from_port);
    tcpheader->destination_port     = switch_endian16(to_port);
    tcpheader->sequence_number      = switch_endian32(sequence_number);
    tcpheader->acknowledge_number   = switch_endian32(acknowledge_number);
    tcpheader->flags                = switch_endian16(flags) + (header_length<<4);
    tcpheader->window_size          = switch_endian16(window);
    tcpheader->checksum             = 0;
    tcpheader->urgent_pointer       = 0;

    int payload = ( size - (sizeof(struct TCPHeader) - sizeof(struct EthernetHeader)) );
    unsigned char *start = (unsigned char*)malloc(sizeof(struct tcp_checksum_header) + payload);
    unsigned char *end = start;
    end += (sizeof(struct tcp_checksum_header));

    struct tcp_checksum_header* trx = (struct tcp_checksum_header*)start;
    trx->dst = (to);
    trx->src = (from);
    trx->len = switch_endian16(20 + payload);
    trx->protocol = IPV4_TYPE_TCP;
    trx->source_port          = switch_endian16(from_port);
    trx->destination_port     = switch_endian16(to_port);
    trx->sequence_number      = switch_endian32(sequence_number);
    trx->acknowledge_number   = switch_endian32(acknowledge_number);
    trx->flags                = switch_endian16(flags) + (header_length<<4);
    trx->window_size          = switch_endian16(window);
    trx->checksum             = 0;
    trx->urgent_pointer       = 0;

    tcpheader->checksum = switch_endian16(net_checksum(start, end));
}

void fillDhcpDiscoverHeader(struct DHCPDISCOVERHeader *dhcpheader){
    unsigned char destmac[SIZE_OF_MAC] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    unsigned short size = sizeof(struct DHCPDISCOVERHeader) - sizeof(struct EthernetHeader);
    fillUdpHeader((struct UDPHeader*)&dhcpheader->udpheader,(unsigned char*)&destmac,size,0,0xFFFFFFFF,68,67);
}

void fillDhcpRequestHeader(struct DHCPREQUESTHeader *dhcpheader){
    unsigned char destmac[SIZE_OF_MAC] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    unsigned short size = sizeof(struct DHCPREQUESTHeader) - sizeof(struct EthernetHeader);
    fillUdpHeader((struct UDPHeader*)&dhcpheader->udpheader,(unsigned char*)&destmac,size,0,0xFFFFFFFF,68,67);
}

unsigned char* getIpAddressFromDHCPServer(){
    struct DHCPDISCOVERHeader *dhcpheader = (struct DHCPDISCOVERHeader *)malloc(sizeof(struct DHCPDISCOVERHeader));
    dhcpheader->op = 1;
    dhcpheader->htype = 1;
    dhcpheader->hlen = 6;
    dhcpheader->hops = 0;
    dhcpheader->xid = 0x26F30339;
    dhcpheader->timing = 0;
    dhcpheader->flags = switch_endian16(0x8000);

    fillMac((unsigned char*)&dhcpheader->client_mac_addr,(unsigned char*)&defaultEthernetDevice.mac);
    dhcpheader->magic_cookie = 0x63538263;
    // DHCP Message Type
    dhcpheader->options[0] = 0x35;
    dhcpheader->options[1] = 0x01;
    dhcpheader->options[2] = 0x01;
    // parameter request list
    dhcpheader->options[3] = 0x37;
    dhcpheader->options[4] = 0x40;
    dhcpheader->options[5] = 0xfc;
    for(unsigned char i = 1 ; i < 0x43 ; i++){
        dhcpheader->options[5+i] = i;
    }
    // dhcpheader->options[68] = 0x00;
    // ip address lease time
    dhcpheader->options[69] = 0x33;
    dhcpheader->options[70] = 0x04;
    dhcpheader->options[71] = 0x00;
    dhcpheader->options[72] = 0x00;
    dhcpheader->options[73] = 0x00;
    dhcpheader->options[74] = 0x01;
    // end
    dhcpheader->options[75] = 0xFF;
    
    fillDhcpDiscoverHeader(dhcpheader);
    PackageRecievedDescriptor sec;
    sec.buffersize = sizeof(struct DHCPDISCOVERHeader);
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)dhcpheader;
    int res_fs = sendEthernetPackage(sec); // send package
    if(res_fs==0){
        return 0;
    }
    PackageRecievedDescriptor prd;
    resetTicks();
    while(1){
        prd = getEthernetPackage(); 
        if(prd.low_buf==0){
            return 0;
        }
        struct EthernetHeader *eh = (struct EthernetHeader*) prd.low_buf;
        if(eh->type==ETHERNET_TYPE_IP4){
            struct DHCPDISCOVERHeader *hd5 = ( struct DHCPDISCOVERHeader*) prd.low_buf;
            if(hd5->options[2]==2&&hd5->xid==dhcpheader->xid&&hd5->op==2){
                break;
            }
        }
    }
    debugf("[ETH] Got offer\n");
    struct DHCPDISCOVERHeader *hd = ( struct DHCPDISCOVERHeader*) prd.low_buf;
    unsigned char* offeredip = (unsigned char*) &hd->dhcp_offered_machine;
    int a = 0;
    while(1){
        unsigned char t = hd->options[a++];
        if(t==0xFF||t==0x00){
            break;
        }
        unsigned char z = hd->options[a++];
        if( t==0x03){ // router
            unsigned char* re = (unsigned char*)&hd->options[a];
            fillIP((unsigned char*)&router_ip,re);
        }
        if (t==0x06 ){ // dns
            unsigned char* re = (unsigned char*)&hd->options[a];
            fillIP((unsigned char*)&dns_ip,re);
        }
        if (t==54 ){ // dhcp
            unsigned char* re = (unsigned char*)&hd->options[a];
            fillIP((unsigned char*)&dhcp_ip,re);
        }
        a += z;
    }

    free(dhcpheader);

    struct DHCPREQUESTHeader *dhcp2header = (struct DHCPREQUESTHeader *)malloc(sizeof(struct DHCPREQUESTHeader));
    dhcp2header->op = 1;
    dhcp2header->htype = 1;
    dhcp2header->hlen = 6;
    dhcp2header->hops = 0;
    dhcp2header->xid = 0x2CF30339;
    dhcp2header->timing = 0;
    dhcp2header->flags = switch_endian16(0x8000);

    fillMac((unsigned char*)&dhcp2header->client_mac_addr,(unsigned char*)&defaultEthernetDevice.mac);
    dhcp2header->magic_cookie = 0x63538263;

    // DHCP Message Type
    dhcp2header->options[0] = 0x35;
    dhcp2header->options[1] = 0x01;
    dhcp2header->options[2] = 0x03;
    // Client identifier
    dhcp2header->options[3] = 0x3d;
    dhcp2header->options[4] = 0x07;
    dhcp2header->options[5] = 0x01;
    fillMac((unsigned char*)(&dhcp2header->options)+6,(unsigned char*)&defaultEthernetDevice.mac);
    // Requested IP addr
    dhcp2header->options[12] = 0x32;
    dhcp2header->options[13] = 0x04;
    fillMac((unsigned char*)(&dhcp2header->options)+14,offeredip);
    // DHCP Server identifier
    dhcp2header->options[18] = 0x36;
    dhcp2header->options[19] = 0x04;
    fillMac((unsigned char*)(&dhcp2header->options)+20,(unsigned char*)&hd->ip_addr_of_dhcp_server);
    dhcp2header->options[24] = 0xFF;

    fillDhcpRequestHeader(dhcp2header);

    PackageRecievedDescriptor s3c;
    s3c.buffersize = sizeof(struct DHCPREQUESTHeader);
    s3c.high_buf = 0;
    s3c.low_buf = (unsigned long)dhcp2header;
    sendEthernetPackage(s3c); // send package
    PackageRecievedDescriptor p3d;
    while(1){
        p3d = getEthernetPackage(); 
        struct EthernetHeader *eh = (struct EthernetHeader*) p3d.low_buf;
        if(eh->type==ETHERNET_TYPE_IP4){
            struct DHCPDISCOVERHeader *hd5 = ( struct DHCPDISCOVERHeader*) p3d.low_buf;
            if(hd5->options[2]==5&&hd5->xid==dhcp2header->xid&&hd5->op==2){
                break;
            }
        } 
    }
    debugf("[ETH] Got Approval\n");

    return offeredip;
}

volatile unsigned short dnstid = 0xe0e0;
unsigned char* getIPFromName(char* name){
    debugf("[ETH] Looking for IP of %s \n",name);
    int str = strlen(name);
    int ourheadersize = sizeof(struct DNSREQUESTHeader)+str+2+4;
    struct DNSREQUESTHeader *dnsreqheader = (struct DNSREQUESTHeader*) malloc(ourheadersize);
    unsigned char *destmac = getMACFromIp((unsigned char*)&dns_ip);
    unsigned short size = ourheadersize - sizeof(struct EthernetHeader);
    dnsreqheader->transaction_id = dnstid++;
    dnsreqheader->flags = 0x1;
    dnsreqheader->question_count = 0x100;
    unsigned char *t4 = (unsigned char*)(dnsreqheader);
    t4[sizeof(struct DNSREQUESTHeader)] = 0;
    int i = 0;
    for(i = 0 ; i < str ; i++){
        t4[sizeof(struct DNSREQUESTHeader)+i+1] = name[i];
    }
    i = 0;
    while( i < (str + 2) ){
        int z = 0;
        for(int t = i+1 ; t < str+2 ; t++){
            unsigned char deze = t4[sizeof(struct DNSREQUESTHeader)+t];
            if(deze==0||deze=='.'){
                break;
            }
            z++;
        }
        t4[sizeof(struct DNSREQUESTHeader)+i] = z;
        i += z + 1;
    }
    t4[sizeof(struct DNSREQUESTHeader)+str+3] = 1;
    t4[sizeof(struct DNSREQUESTHeader)+str+5] = 1;
    
    fillUdpHeader((struct UDPHeader*)&dnsreqheader->udpheader,destmac,size,((unsigned long*)&our_ip)[0],((unsigned long*)&dns_ip)[0],56331,53);
    
    PackageRecievedDescriptor sec;
    sec.buffersize = ourheadersize;
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)dnsreqheader;
    sendEthernetPackage(sec);
    struct DNSREQUESTHeader* de;
    PackageRecievedDescriptor ep;
    unsigned char* targetip = (unsigned char*) malloc(SIZE_OF_IP);
    while(1){
        ep = getEthernetPackage();
        if(ep.low_buf==0){
            debugf("[ETH] IP of %s cannot be found \n",name);
            return targetip;
        }
        de = (struct DNSREQUESTHeader*) ep.low_buf;
        if(de->transaction_id==dnsreqheader->transaction_id){
            break;
        }
        ethernet_handle_package(ep);
    }
    if(switch_endian16(de->answer_rr)>0){
        targetip[0] = ((unsigned char*)de + (ep.buffersize-4))[0];
        targetip[1] = ((unsigned char*)de + (ep.buffersize-3))[0];
        targetip[2] = ((unsigned char*)de + (ep.buffersize-2))[0];
        targetip[3] = ((unsigned char*)de + (ep.buffersize-1))[0];
    }
    debugf("[ETH] IP of %s is %d.%d.%d.%d \n",name,targetip[0],targetip[1],targetip[2],targetip[3]);
    return targetip; 
}

unsigned long ethjmplist[20000];

void setTcpHandler(unsigned short port,unsigned long func){
    ethjmplist[port] = func;
}

void create_tcp_session(unsigned long from, unsigned long to, unsigned short from_port, unsigned short to_port, unsigned long func){
    unsigned long sizetype = sizeof(struct TCPHeader);
    struct TCPHeader* tcp1 = (struct TCPHeader*) malloc(sizetype);
    unsigned char* destmac;
    unsigned char* t4 = (unsigned char*)&to;

    if(t4[0]==192){
        destmac = getMACFromIp(t4);
    }else{
        destmac = getMACFromIp((unsigned char*)&router_ip);
    }
    unsigned short size = sizeof(struct TCPHeader) - sizeof(struct EthernetHeader);
    fillTcpHeader(tcp1,destmac,size,from,to,from_port,to_port,1,0,5,TCP_SYN,0xffd7);

    setTcpHandler(to_port,func);

    PackageRecievedDescriptor sec;
    sec.buffersize = sizetype;
    sec.high_buf = 0;
    sec.low_buf = (unsigned long)tcp1;
    sendEthernetPackage(sec);
}

int ethernet_handle_package(PackageRecievedDescriptor desc){
    struct EthernetHeader *eh = (struct EthernetHeader*) desc.low_buf;
    if(eh->type==ETHERNET_TYPE_ARP){
        struct ARPHeader *ah = (struct ARPHeader*) desc.low_buf;
        if( ah->operation==0x0100 && memcmp((char*) ah->dest_ip,(char*) &our_ip, SIZE_OF_IP)==0 ){
            debugf("[ETH] ARP recieved with our IP\n");

            struct ARPHeader* arpie = (struct ARPHeader*)malloc(sizeof(struct ARPHeader));
            unsigned char everyone[SIZE_OF_MAC] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
            fillEthernetHeader((struct EthernetHeader*) &arpie->ethernetheader,everyone,ETHERNET_TYPE_ARP);
            arpie->hardware_type = 0x0100;
            arpie->protocol_type = 0x0008;
            arpie->hardware_address_length = SIZE_OF_MAC;
            arpie->protocol_address_length = SIZE_OF_IP;
            arpie->operation = 0x0200;

            fillMac((unsigned char*)&arpie->source_mac,(unsigned char*)&defaultEthernetDevice.mac);
            fillIP((unsigned char*)&arpie->source_ip,(unsigned char*)&our_ip);

            fillMac((unsigned char*)&arpie->dest_mac,(unsigned char*)ah->source_mac);
            fillIP((unsigned char*)&arpie->dest_ip,(unsigned char*)ah->source_ip);
            
            PackageRecievedDescriptor sec;
            sec.buffersize = sizeof(struct ARPHeader);
            sec.high_buf = 0;
            sec.low_buf = (unsigned long)arpie;

            sendEthernetPackage(sec);
            return 1;
        }else if(ah->operation==0x0100){
            return 1;
        }
    }else if(eh->type==ETHERNET_TYPE_IP4){
        struct IPv4Header* ip = (struct IPv4Header*) eh;
        if(ip->protocol==IPV4_TYPE_UDP){
            struct UDPHeader* udp = (struct UDPHeader*) eh;
            debugf("[ETH] UDP package recieved for port %x !\n",switch_endian16(udp->destination_port));
            if(udp->destination_port==switch_endian16(50618)){
                // TFTP, automatic ACK
                struct TFTPAcknowledgeHeader* tftp_old = (struct TFTPAcknowledgeHeader*)eh;
                struct TFTPAcknowledgeHeader* tftp = (struct TFTPAcknowledgeHeader*)malloc(sizeof(struct TFTPAcknowledgeHeader));

                tftp->index = tftp_old->index;
                tftp->type = switch_endian16(4);

                unsigned short packagelength = sizeof(struct UDPHeader) + 4 ;
                fillUdpHeader((struct UDPHeader*)&tftp->header,(unsigned char*)tftp_old->header.ipv4header.ethernetheader.from,packagelength-sizeof(struct EthernetHeader),getOurIpAsLong(),tftp_old->header.ipv4header.source_addr,50618,switch_endian16(tftp_old->header.source_port));
            
                PackageRecievedDescriptor sec;
                sec.buffersize = sizeof(struct TFTPAcknowledgeHeader);
                sec.high_buf = 0;
                sec.low_buf = (unsigned long)tftp;

                sendEthernetPackage(sec);

            }
        }else if(ip->protocol==IPV4_TYPE_TCP){
            struct TCPHeader* tcp = (struct TCPHeader*) eh;
            unsigned short fx = switch_endian16(tcp->flags);
            debugf("[ETH] TCP package recieved for port %x %s %s %s %s !\n",switch_endian16(tcp->destination_port),fx&TCP_PUS?"PUSH":"",fx&TCP_SYN?"SYN":"",fx&TCP_ACK?"ACK":"",fx&TCP_FIN?"FIN":"");
            if(((switch_endian16(tcp->flags) & TCP_PUS)||(switch_endian16(tcp->flags) & TCP_SYN)||(switch_endian16(tcp->flags) & TCP_FIN)) && (switch_endian16(tcp->flags) & TCP_ACK)){
                // TCP auto accept ACK SYN
                debugf("[ETH] TCP package handled\n");
                unsigned long from = tcp->header.dest_addr; 
                unsigned long to = tcp->header.source_addr; 
                unsigned short from_port = switch_endian16(tcp->destination_port); 
                unsigned short to_port = switch_endian16(tcp->source_port);
                unsigned long sizetype = sizeof(struct TCPHeader);
                struct TCPHeader* tcp1 = (struct TCPHeader*) malloc(sizetype);
                unsigned char* destmac = (unsigned char*)tcp->header.ethernetheader.from;
                unsigned short size = sizeof(struct TCPHeader) - sizeof(struct EthernetHeader);
                unsigned long sid = switch_endian32(tcp->sequence_number);
                if(switch_endian16(tcp->flags) & TCP_PUS){
                    unsigned long tr = desc.buffersize - sizeof(struct TCPHeader);
                    sid += tr;
                }else if(switch_endian16(tcp->flags) & TCP_SYN){
                    sid++;
                }else if(switch_endian16(tcp->flags) & TCP_FIN){
                    sid++;
                }
                fillTcpHeader(tcp1,destmac,size,from,to,from_port,to_port,switch_endian32(tcp->acknowledge_number),sid,5,TCP_ACK,512);

                PackageRecievedDescriptor sec;
                sec.buffersize = sizetype;
                sec.high_buf = 0;
                sec.low_buf = (unsigned long)tcp1;
                sendEthernetPackage(sec);

                if(switch_endian16(tcp->flags) & TCP_PUS){
                    unsigned long addr = desc.low_buf + sizeof(struct TCPHeader);
                    unsigned long count = desc.buffersize-sizeof(struct TCPHeader);
                    unsigned long func = ethjmplist[switch_endian16(tcp->destination_port)];
                    debugf("[ETH] TCP message reieved: size=%x string=%s \n",count,(unsigned char*)addr);
                    if(func){
                        debugf("[ETH] function handler is about to get called\n");
                        int (*sendPackage)(unsigned long a,unsigned long b) = (void*)func;
                        sendPackage(addr,count);
                    }else{
                        debugf("[ETH] No function handler for this tcpservice!\n");
                    }
                }
                if(switch_endian16(tcp->flags) & TCP_FIN){
                    debugf("[ETH] Stream is finished!\n");
                }
            }
            return 1;
        }else if(ip->protocol==IPV4_TYPE_ICMP){
            struct ICMPHeader *icmp = (struct ICMPHeader*) ip;
            if(icmp->type==8){
                debugf("[ETH] ICMP ping request found!\n");

                int prefsiz = desc.buffersize - sizeof(struct ICMPHeader);
                struct ICMPHeader *newicmp = (struct ICMPHeader*) malloc(sizeof(struct ICMPHeader) + prefsiz );
                unsigned short size = (sizeof(struct ICMPHeader) + prefsiz) - sizeof(struct EthernetHeader);
                fillIpv4Header((struct IPv4Header*)&newicmp->ipv4header,(unsigned char*)icmp->ipv4header.ethernetheader.from,size,IPV4_TYPE_ICMP,icmp->ipv4header.dest_addr,icmp->ipv4header.source_addr);
                unsigned char *tty = (unsigned char *)icmp;
                unsigned char *tt0 = (unsigned char *)newicmp;
                for(unsigned int i = 0 ; i < ((sizeof(struct ICMPHeader) + prefsiz) - sizeof(struct IPv4Header)) ; i++){
                    tt0[sizeof(struct IPv4Header)+i] = tty[sizeof(struct IPv4Header)+i];
                }
                newicmp->type = 0;

                unsigned short tu = switch_endian16(~icmp->checksum);
                tu -= 0x800;
                newicmp->checksum = switch_endian16(~tu);
                
                PackageRecievedDescriptor sec;
                sec.buffersize = sizeof(struct ICMPHeader) + prefsiz;
                sec.high_buf = 0;
                sec.low_buf = (unsigned long)newicmp;
                sendEthernetPackage(sec);
                return 1;
            }
        }
    }
    return 0;
}

unsigned long getOurIpAsLong(){
    return ((unsigned long*)&our_ip)[0];
}

void exsend(unsigned long addr,unsigned long count){
    printf("Recieved message: ");
    for(unsigned long i = 0 ; i < count ; i++){
        printf("%c",((unsigned char*)addr)[i]);
    }
    printf("\n");
}

void initialise_ethernet(){
    printf("[ETH] Ethernet module reached!\n");
    EthernetDevice ed = getDefaultEthernetDevice();
    if(ed.is_enabled){
        printf("[ETH] There is a ethernet device present on the system!\n");
        printf("[ETH] Asking DHCP server for our address....\n");

        unsigned char *dhcpid = getIpAddressFromDHCPServer();
        if(dhcpid){
            fillIP((unsigned char*)&our_ip,dhcpid);
            printf("[ETH] DHCP is present\n");
        }else{
            printf("[ETH] No DHCP server present here, using static address\n");
            unsigned char dinges[SIZE_OF_IP] = {192,168,178,15};   
            fillIP((unsigned char*)&our_ip,(unsigned char*)&dinges);
        }

        printf("[ETH] Our     IP is %d.%d.%d.%d \n",our_ip[0],our_ip[1],our_ip[2],our_ip[3]);
        printf("[ETH] Gateway IP is %d.%d.%d.%d \n",router_ip[0],router_ip[1],router_ip[2],router_ip[3]);
        printf("[ETH] DNS     IP is %d.%d.%d.%d \n",dns_ip[0],dns_ip[1],dns_ip[2],dns_ip[3]);
        printf("[ETH] DHCP    IP is %d.%d.%d.%d \n",dhcp_ip[0],dhcp_ip[1],dhcp_ip[2],dhcp_ip[3]);

        debugf("[ETH] Our     IP is %d.%d.%d.%d \n",our_ip[0],our_ip[1],our_ip[2],our_ip[3]);
        debugf("[ETH] Gateway IP is %d.%d.%d.%d \n",router_ip[0],router_ip[1],router_ip[2],router_ip[3]);
        debugf("[ETH] DNS     IP is %d.%d.%d.%d \n",dns_ip[0],dns_ip[1],dns_ip[2],dns_ip[3]);
        debugf("[ETH] DHCP    IP is %d.%d.%d.%d \n",dhcp_ip[0],dhcp_ip[1],dhcp_ip[2],dhcp_ip[3]);

        if(dns_ip[0]){
            unsigned char* srve = getIPFromName("tftp.local");
            if(srve[0]){
                printf("[ETH] TFTP    IP is %d.%d.%d.%d \n",srve[0],srve[1],srve[2],srve[3]);
                debugf("[ETH] TFTP    IP is %d.%d.%d.%d \n",srve[0],srve[1],srve[2],srve[3]);
                unsigned char ipfs[SIZE_OF_IP];
                ipfs[0] = dhcp_ip[0];
                ipfs[1] = dhcp_ip[1];
                ipfs[2] = dhcp_ip[2];
                ipfs[3] = dhcp_ip[3];
                Device *dev = getNextFreeDevice();
                dev->arg4 = (unsigned long)&ipfs;
                dev->arg5 = (unsigned long)getMACFromIp((unsigned char*)&ipfs);
                initialiseTFTP(dev);
            }
        }
    }
}