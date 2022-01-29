#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <string.h>
#include <symbols.h>

#define DEFAULT_IRC_PORT 6667
#define DEFAULT_IRC_ADDR "chat.freenode.net"

volatile unsigned char w1 = 0;

unsigned char* getIPFromName(char* t);

void recievemessage(unsigned long addr,unsigned long count){
    write(1, (const char *)addr, count);
    unsigned char t[2];
    t[0] = '\n';
    t[1] = 0x00;
    write(1, (const char *)&t, 1);
}

void sendUsername(char* username){}
void sendNickname(char* nickname){}

int main(int argc, char** argv){
    printf("welcome to our irc client\n");
    unsigned char *t = getIPFromName(DEFAULT_IRC_ADDR);//(unsigned char*) DEFAULT_IRC_ADDR;
    int tx = 0x00200000;
    initialisenetworkpackage(2,1,t,tx,DEFAULT_IRC_PORT);
    for(;;);
    return EXIT_SUCCESS;
}

unsigned char* getIPFromName(char* t){
    unsigned long func = F_GETIPFROMNAME;
    unsigned char* (*sendPackage)(char* t) = (void*)func;
    return sendPackage(t);
}