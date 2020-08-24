#include "../kernel.h"

// info: https://wiki.osdev.org/Sound_Blaster_16
// declaraties
#define SOUNDBLASTER16_DSP_MIXER_PORT 0x224
#define SOUNDBLASTER16_DSP_MIXER_DATA_PORT 0x225
#define SOUNDBLASTER16_DSP_RESET 0x226
#define SOUNDBLASTER16_DSP_READ 0x22A
#define SOUNDBLASTER16_DSP_WRITE 0x22C
#define SOUNDBLASTER16_DSP_READ_STATUS 0x22E 
#define SOUNDBLASTER16_DSP_16BACK 0x22F

extern void sndirq();
volatile unsigned char soundblaster16_irq_fire = 0;
void irq_snd(){
	soundblaster16_irq_fire = 1;
	unsigned char tp1 = inportb(SOUNDBLASTER16_DSP_READ_STATUS);
	unsigned char tp2 = inportb(SOUNDBLASTER16_DSP_16BACK);
	printf("[SNDBST] Interrupt fired with 1=%x 2=%x \n",tp1,tp2);
	outportb(0xA0,0x20);
	outportb(0x20,0x20);
}

void soundblaster16_reset(){
	outportb(SOUNDBLASTER16_DSP_RESET,1);
	sleep(3);
	outportb(SOUNDBLASTER16_DSP_RESET,0);
}

int soundblaster16_detect(){
	soundblaster16_reset();
	unsigned char resultaat = inportb(SOUNDBLASTER16_DSP_READ);
	printf("[SNDBST] Result reset=%x (Should be 0xAA)\n",resultaat);
	return resultaat==0xAA;
}

void soundblaster16_setVolume(unsigned char volume){
	outportb(SOUNDBLASTER16_DSP_MIXER_PORT,0x22);
	outportb(SOUNDBLASTER16_DSP_MIXER_PORT,volume);
}

int soundblaster16_playSoundData(void *data,unsigned short size,unsigned char hertz,unsigned char mono,unsigned long bittransfers){

	// speaker aanzetten
	outportb(SOUNDBLASTER16_DSP_WRITE,0xD1);

	// DMA activeren
	if(bittransfers==8){
		outportb(0x0A,5);
		outportb(0x0C,1);
		outportb(0x0B,0x49);
		outportb(0x83,(((unsigned long)data)>>16)&0xFF);
		outportb(0x02,((unsigned long)data)&0xFF);
		outportb(0x02,(((unsigned long)data)>>8)&0xFF);
		outportb(0x03,size&0xFF);
		outportb(0x03,(size>>8)&0xFF);
		outportb(0x0A,1);
	}else if(bittransfers==16){
		return 0;
	}else{
		return 0;
	}

	// IRQ reset
	soundblaster16_irq_fire = 0;

	// TIME constant instellen
	outportb(SOUNDBLASTER16_DSP_WRITE,0x40);
	outportb(SOUNDBLASTER16_DSP_WRITE,hertz);

	outportb(SOUNDBLASTER16_DSP_WRITE,bittransfers==8?0xC0:0xB0);

	// transfermode instellen
	outportb(SOUNDBLASTER16_DSP_WRITE,0x00);

	// type aangeven
	outportb(SOUNDBLASTER16_DSP_WRITE,mono==0?0x20:0x00);

	// data lengte aangeven
	outportb(SOUNDBLASTER16_DSP_WRITE,size&0xFF);
	outportb(SOUNDBLASTER16_DSP_WRITE,(size>>8)&0xFF);

	// geluid afspelen
	again:
	if(soundblaster16_irq_fire==0){
		goto again;
	}

	// geluid stoppen
	outportb(SOUNDBLASTER16_DSP_WRITE,0xD3);

	// stoppen
	return 1;
}

int soundblaster16_playSampleData(){
	unsigned short size = 0x0005;
	for(unsigned short i = 0 ; i < size ; i++){
		((unsigned char*)0x1000)[i] = 0xCD;
	}
	soundblaster16_setVolume(0xFF);
	return soundblaster16_playSoundData((void *)0x1000,size,165,1,8);
}

void init_soundblaster16(){
	
	//
	// detect and reset
	printf("[SNDBST] Detecting possible existance of a soundblaster16 device\n");
	if(soundblaster16_detect()==0){
		printf("[SNDBST] No soundblaster available\n");
		return;
	}
	printf("[SNDBST] Soundblaster available\n");

	//
	// set interrupt
	unsigned char targetinterrupt = 5;
	printf("[SNDBST] Setting interrupt to %x \n",targetinterrupt);
	setNormalInt(targetinterrupt,(unsigned long)sndirq);
	outportb(SOUNDBLASTER16_DSP_MIXER_PORT,0x80);
	outportb(SOUNDBLASTER16_DSP_MIXER_PORT,0x02); // 2= interrupt 5

	int i = soundblaster16_playSampleData();
	if(i){
		printf("[SNDBST] Finished playing example\n");
	}else{
		printf("[SNDBST] Unable to playing example\n");
	}

}