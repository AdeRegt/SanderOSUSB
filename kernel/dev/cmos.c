#include "../kernel.h"

// from: https://wiki.osdev.org/CMOS

#define CMOS_PORT_ADDRESS 0x70
#define CMOS_PORT_DATA 0x71

unsigned char cmos_read(unsigned char port){
    outportb(CMOS_PORT_ADDRESS,port);
    return inportb(CMOS_PORT_DATA);
}

unsigned char cmos_get_update_in_progress_flag(){
    return cmos_read(0x0A) & 0x80;
}

unsigned char* cmos_update_datetime_to_variables(){
    while(cmos_get_update_in_progress_flag());
    unsigned char* data = (unsigned char*) malloc(6);
    data[0] = cmos_read(0);
    data[1] = cmos_read(2);
    data[2] = cmos_read(4);
    data[3] = cmos_read(7);
    data[4] = cmos_read(8);
    data[5] = cmos_read(9);
    return data;
}

void init_cmos(){
    debugf("CMOS: ready!\n");
    unsigned char *dataset = cmos_update_datetime_to_variables();
    debugf("CMOS: %x:%x:%x %x/%x/%x \n",dataset[0],dataset[1],dataset[2],dataset[3],dataset[4],2000+dataset[5]);
    free(dataset);
}