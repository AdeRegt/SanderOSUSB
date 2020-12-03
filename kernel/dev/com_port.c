#include "../kernel.h"
//
// SERIAL
//
//

extern void serialirq();


unsigned int serial_received(unsigned short PORT) {
   return inportb(PORT + 5) & 1;
}
 
unsigned char read_serial(unsigned short PORT) {
   while (serial_received(PORT) == 0);
 
   return inportb(PORT);
}

int is_transmit_empty(unsigned short PORT) {
   return inportb(PORT + 5) & 0x20;
}
 
void write_serial(char a,unsigned short PORT) {
   while (is_transmit_empty(PORT) == 0);
 
   outportb(PORT,a);
}

static unsigned char last_com_char = 0;

void irq_serial(){
   unsigned short port = 0;
   if(serial_received(0x3f8)){
      port = 0x3f8;
   }else if(serial_received(0x2f8)){
      port = 0x2f8;
   }else if(serial_received(0x3e8)){
      port = 0x3e8;
   }else if(serial_received(0x2e8)){
      port = 0x2e8;
   }else{
      printf("[SER] Input via unknown port\n");
      goto out;
   }
   printf("[SER] Input via serial via port %x !\n",port);
	unsigned char binnengekomen = read_serial(port);
   last_com_char = binnengekomen;
   out:
	outportb(0x20, 0x20);
	outportb(0xA0, 0x20);
}

void init_serial_device(unsigned short PORT) {
   outportb(PORT + 1, 0x00);    // Disable all interrupts
   outportb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outportb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outportb(PORT + 1, 0x00);    //                  (hi byte)
   outportb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outportb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outportb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outportb(PORT + 1, 1);
   setNormalInt(4,(unsigned long)serialirq);
   printf("[SER] Serial %x installed!\n",PORT);
}

void init_serial(){
	init_serial_device(0x3f8);
	init_serial_device(0x2f8);
	init_serial_device(0x3e8);
	init_serial_device(0x2e8);
}
