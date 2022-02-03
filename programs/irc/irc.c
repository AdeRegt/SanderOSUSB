#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <string.h>
#include <symbols.h>

#define DEFAULT_IRC_PORT 6667
#define DEFAULT_IRC_ADDR "chat.freenode.net"
#define MESSAGEBUFFER 50

unsigned char *addressip;
unsigned char* getIPFromName(char* t);

void recievemessage(unsigned long addr,unsigned long count){
    write(1, (const char *)addr, count);
    unsigned char* buffer = (unsigned char*) addr;
    if(buffer[0]=='P'&&buffer[1]=='I'&&buffer[2]=='N'&&buffer[3]=='G'&&buffer[4]==' '){
        buffer[1] = 'O';
        unsigned char *t4 = (unsigned char*)0x1000;
        sendnetworkpackage(2,count,t4,(int)buffer,DEFAULT_IRC_PORT);
    }
}

char* getline(){
    char* message = malloc(MESSAGEBUFFER);
    int i = 0;
    for(i = 0 ; i < MESSAGEBUFFER ; i++){
        unsigned char y = getc((FILE*)1) & 0x000000FF;
        message[i] = y;
        write(1, (void *)&message[i], 1);
        if(y=='\n'){
            message[i+1] = 0;
            break;
        }
    }
    message[i+1] = 0;
    return message;
}

int main(int argc, char** argv){
    printf("welcome to our irc client\n");
    addressip = getIPFromName(DEFAULT_IRC_ADDR);
    
    unsigned char *t4 = (unsigned char*)0x1000;
    t4[0] = addressip[0];
    t4[1] = addressip[1];
    t4[2] = addressip[2];
    t4[3] = addressip[3];
    int tx = 0x00200000;
    initialisenetworkpackage(2,1,addressip,tx,DEFAULT_IRC_PORT);
    while(1){
        char *z = getline();
        size_t sizeline = strlen(z);
        for(size_t i = 0 ; i < sizeline ; i++){
            char y = z[i];
            if(y==' '||y==0x0a){
                break;
            }
            y += ('A'-'a');
            z[i] = y;
        }
        for(size_t i = 0 ; i < sizeline ; i++){
            if(z[i]==';'){
                z[i] = ':';
            }
            if(z[i]=='\\'){
                z[i] = '#';
            }
        }
        sendnetworkpackage(2,sizeline,t4,(int)z,DEFAULT_IRC_PORT);
        free(z);
    }
    return EXIT_SUCCESS;
}

unsigned char* getIPFromName(char* t){
    unsigned long func = F_GETIPFROMNAME;
    unsigned char* (*sendPackage)(char* t) = (void*)func;
    return sendPackage(t);
}