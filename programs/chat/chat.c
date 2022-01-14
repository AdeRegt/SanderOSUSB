#include <stdio.h>
#include <stdlib.h>
#include <network.h>

#define MESSAGEBUFFER 25

void *buffer;
unsigned char ipaddr[SIZE_OF_IP] = {192,168,2,92};

void clearbuffer(){
    for(int i = 0 ; i < MESSAGEBUFFER ; i++){((unsigned char*)buffer)[i]=0;}
}

void getline(){
    clearbuffer();
    for(int i = 0 ; i < MESSAGEBUFFER ; i++){
        unsigned char y = getc((FILE*)1);
        ((unsigned char*)buffer)[i] = y;
        write(1, buffer+i, 1);
        if(y=='\n'){
            break;
        }
    }
}

void *getValidPackage(){
    void *edx;
    while(1){
        edx = getnetworkpackage();
        struct EthernetHeader* eh = (struct EthernetHeader*) edx;
        if(eh->type==ETHERNET_TYPE_IP4){
            struct IPv4Header* ip = (struct IPv4Header*) edx;
            if(ip->protocol==17){
                struct UDPHeader* udp = (struct UDPHeader*) edx;
                if(udp->destination_port==0xCB1){
                    break;
                }
            }
        }
    }
    return edx;
}

void handlePackage(void *edx){
    clearbuffer();
    for(int i = 0 ; i < MESSAGEBUFFER ; i++){
        ((unsigned char*)buffer)[i]=((unsigned char*)edx)[i+sizeof(struct UDPHeader)];
        if(((unsigned char*)buffer)[i]==0){
            ((unsigned char*)buffer)[i] = '\n';
        }
    }
    write(1, buffer, MESSAGEBUFFER);
    ((unsigned char*)buffer)[0] = '\n';
    write(1,(unsigned char*)(buffer),1);
}

void tick(){
    getline();
    sendnetworkpackage(1,MESSAGEBUFFER,(unsigned char*)&ipaddr,buffer,45324);
    void* edx = getValidPackage();
    handlePackage(edx);
}

int main(int argc, char** argv){
    printf("Im ready for it!\n");
    buffer = malloc(MESSAGEBUFFER);
    while(1){
        tick();
    }
    return EXIT_SUCCESS;
}