#include <stdio.h>
#include <stdlib.h>
#include <network.h>

#define MESSAGEBUFFER 25
#define PORT 20001

volatile unsigned char buffer[MESSAGEBUFFER];

void clearbuffer(){
    for(int i = 0 ; i < MESSAGEBUFFER ; i++){buffer[i]=0;}
}

int main(int argc, char** argv){
    printf("Im ready for it!\nEnter the target ip followed by the port:");
    char* toz = (char*) malloc(12);
    char* t0z = (char*) malloc(2);
    t0z[0] = '.';
    t0z[1] = 0;
    volatile unsigned char torry[SIZE_OF_IP];
    readstring(toz,3);
    torry[0] = strtol(toz,NULL,10);
    write(1,(const char*)(t0z),1);
    readstring(toz,3);
    torry[1] = strtol(toz,NULL,10);
    write(1,(const char*)(t0z),1);
    readstring(toz,3);
    torry[2] = strtol(toz,NULL,10);
    write(1,(const char*)(t0z),1);
    readstring(toz,3);
    torry[3] = strtol(toz,NULL,10);
    t0z[0] = '@';
    write(1,(const char*)(t0z),1);
    readstring(toz,5);
    int port = strtol(toz,NULL,10);
    t0z[0] = '\n';
    t0z[1] = 0;
    write(1,(const char*)(t0z),1);
    while(1){
        clearbuffer();
        for(int i = 0 ; i < MESSAGEBUFFER ; i++){
            unsigned char y = getc((FILE*)1) & 0x000000FF;
            buffer[i] = y;
            write(1, (void *)&buffer[i], 1);
            if(y=='\n'){
                buffer[i+1] = 0;
                break;
            }
        }
        unsigned char ipaddr[SIZE_OF_IP] = {torry[0],torry[1],torry[2],torry[3]};
        sendnetworkpackage(1,MESSAGEBUFFER,(unsigned char*)&ipaddr,(int)(&buffer),port);
        void *edx;
        while(1){
            edx = getnetworkpackage();
            struct EthernetHeader* eh = (struct EthernetHeader*) edx;
            if(eh->type==ETHERNET_TYPE_IP4){
                struct IPv4Header* ip = (struct IPv4Header*) edx;
                if(ip->protocol==17){
                    struct UDPHeader* udp = (struct UDPHeader*) edx;
                    if(udp->destination_port==switch_endian16(port)){
                        break;
                    }
                }
            }
        }
        clearbuffer();
        for(int i = 0 ; i < MESSAGEBUFFER ; i++){
            buffer[i]=((unsigned char*)edx)[i+sizeof(struct UDPHeader)];
            if(buffer[i]==0){
                buffer[i] = '\n';
                buffer[i+1] = '\0';
                break;
            }
        }
        write(1, (void*)&buffer, MESSAGEBUFFER);
        buffer[0] = '\n';
        write(1,(const char*)(&buffer),1);
    }
    return EXIT_SUCCESS;
}