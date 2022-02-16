;
; Requires the SoS Development Kit
; 

	format	ELF
	public	_start
	extrn main
	
section '.text' executable
	
_start:
;	push _char_c
;	push 5
	; get params
	mov eax,0xCB
	int 0x80
	push eax 
	push ebx
	; call main
	call main
	; quit
	mov eax,0x01
	int 0x80
	ret

filenamebuffer: db 'A@PRGS/TEGT.ASM', 0
fileoutbuffer: db 'A@PRGS/TEGT.BIN', 0
fasm: db './fasm', 0
dashm: db '-m', 0
ten: db '10', 0
_char_c dd fasm, filenamebuffer, fileoutbuffer, dashm, ten