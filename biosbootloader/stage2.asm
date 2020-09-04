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

mov si,message
call print_string

cli
hlt

;
; Prints a string on the screen
; IN: SI the string to print
;
print_string:
	pusha
	mov ah,0x0E
	.again:
		lodsb
		cmp al,0x00
		je .done
		int 0x10
		jmp short .again
	.done:
	popa
	ret

message db "Hello from stage2!",0x00