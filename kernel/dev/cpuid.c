#include "../kernel.h"
#include "cpuid.h"

char* cpuid_get_model(){
    int buffer1 = 0;
    int buffer2 = 0;
    int buffer3 = 0;
    int nothing = 0;
    __cpuid(0,nothing,buffer1,buffer3,buffer2);
    char *strbuf = "            ";
    strbuf[0] = buffer1 & 0xFF;
    strbuf[1] = (buffer1>>8) & 0xFF;
    strbuf[2] = (buffer1>>16) & 0xFF;
    strbuf[3] = (buffer1>>24) & 0xFF;
    strbuf[4] = buffer2 & 0xFF;
    strbuf[5] = (buffer2>>8) & 0xFF;
    strbuf[6] = (buffer2>>16) & 0xFF;
    strbuf[7] = (buffer2>>24) & 0xFF;
    strbuf[8] = buffer3 & 0xFF;
    strbuf[9] = (buffer3>>8) & 0xFF;
    strbuf[10] = (buffer3>>16) & 0xFF;
    strbuf[11] = (buffer3>>24) & 0xFF;
    return strbuf;
}

void cpuid_get_details(){
    debugf("CPUVendor: \"%s\" \n",cpuid_get_model());
}