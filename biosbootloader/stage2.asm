BITS 16

cli
mov ax, 0
mov ss, ax
mov sp, 0FFFFh
sti

cld				
				

mov ax, 2000h
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax

mov si,0x1000
mov al,byte [si]
mov byte [_bootdevice], al

call _main

%include "stage2c.asm"
