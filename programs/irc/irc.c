#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <string.h>

#define DEFAULT_IRC_PORT 6667

volatile unsigned char w1 = 0;

void recievemessage(unsigned long addr,unsigned long count){
    write(1, (const char *)addr, count);
    unsigned char t[2];
    t[0] = '\n';
    t[1] = 0x00;
    write(1, (const char *)&t, 1);

    if(((volatile unsigned char*)&w1)[0]==0){
        unsigned char* setnicname = (unsigned char*)malloc(12);
        setnicname[0] = 'N';
        setnicname[1] = 'I';
        setnicname[2] = 'C';
        setnicname[3] = 'K';
        setnicname[4] = ' ';
        setnicname[5] = 'S';
        setnicname[6] = 'A';
        setnicname[7] = 'N';
        setnicname[8] = 'D';
        setnicname[9] = 'E';
        setnicname[10] = 'R';
        unsigned char *y = malloc(SIZE_OF_IP);
        y[0] = 192;
        y[1] = 168;
        y[2] = 2;
        sendnetworkpackage(2,strlen((const char*)setnicname),y,(int)setnicname,DEFAULT_IRC_PORT);
    }
    ((volatile unsigned char*)&w1)[0] = 1;
}

void sendUsername(char* username){}
void sendNickname(char* nickname){}

int main(int argc, char** argv){
    printf("welcome to our irc client\n");
    unsigned char *t = malloc(SIZE_OF_IP);
    t[0] = 192;
    t[1] = 168;
    t[2] = 2;
    int tx = 0x00200000;
    initialisenetworkpackage(2,1,t,tx,DEFAULT_IRC_PORT);
    for(;;);
    return EXIT_SUCCESS;
}