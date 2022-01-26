
#define SIZE_OF_MAC 6
#define SIZE_OF_IP 4
#define ETHERNET_TYPE_ARP 0x0608
#define ETHERNET_TYPE_IP4 0x0008

struct EthernetHeader{
    unsigned char to[SIZE_OF_MAC];
    unsigned char from[SIZE_OF_MAC];
    unsigned short type;
} __attribute__ ((packed));

struct ARPHeader{
    struct EthernetHeader ethernetheader;
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_address_length;
    unsigned char protocol_address_length;
    unsigned short operation;

    unsigned char source_mac[SIZE_OF_MAC];
    unsigned char source_ip[SIZE_OF_IP];

    unsigned char dest_mac[SIZE_OF_MAC];
    unsigned char dest_ip[SIZE_OF_IP];
} __attribute__ ((packed));

struct IPv4Header{
    struct EthernetHeader ethernetheader;
    unsigned char internet_header_length:4;
    unsigned char version:4;
    unsigned char type_of_service;
    unsigned short total_length;
    unsigned short id;
    unsigned short flags:3;
    unsigned short fragment_offset:13;
    unsigned char time_to_live;
    unsigned char protocol;
    unsigned short checksum;
    unsigned long source_addr;
    unsigned long dest_addr;
} __attribute__ ((packed));

struct UDPHeader{
    struct IPv4Header ipv4header;
    unsigned short source_port;
    unsigned short destination_port;
    unsigned short length;
    unsigned short checksum;
} __attribute__ ((packed));

struct DHCPDISCOVERHeader{
    struct UDPHeader udpheader;
    unsigned char op;
    unsigned char htype;
    unsigned char hlen;
    unsigned char hops;
    unsigned long xid;
    unsigned short timing;
    unsigned short flags;
    unsigned long address_of_machine;
    unsigned long dhcp_offered_machine;
    unsigned long ip_addr_of_dhcp_server;
    unsigned long ip_addr_of_relay;
    unsigned char client_mac_addr [16];
    unsigned char sname [64];
    unsigned char file [128];
    unsigned long magic_cookie;
    unsigned char options[76];
} __attribute__ ((packed));

struct DHCPREQUESTHeader{
    struct UDPHeader udpheader;
    unsigned char op;
    unsigned char htype;
    unsigned char hlen;
    unsigned char hops;
    unsigned long xid;
    unsigned short timing;
    unsigned short flags;
    unsigned long address_of_machine;
    unsigned long dhcp_offered_machine;
    unsigned long ip_addr_of_dhcp_server;
    unsigned long ip_addr_of_relay;
    unsigned char client_mac_addr [16];
    unsigned char sname [64];
    unsigned char file [128];
    unsigned long magic_cookie;
    unsigned char options[25];
} __attribute__ ((packed));

struct DNSREQUESTHeader{
    struct UDPHeader udpheader;
    unsigned short transaction_id;
    unsigned short flags;
    unsigned short question_count;
    unsigned short answer_rr;
    unsigned short authority_rr;
    unsigned short aditional_rr;
} __attribute__ ((packed));


void *getnetworkpackage();
void sendnetworkpackage(int type,int size,unsigned char *to,int where,int port);
char initialisenetworkpackage(int type,int is_ip,unsigned char *to, int function, int port);