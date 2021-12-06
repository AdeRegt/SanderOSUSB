;
; Requires the SoS Development Kit
; 

	format	ELF
	public	_start
	extrn main
	extrn exit
	
section '.text' executable
	
_start:
	push _char_c
	push 5
	call main
	; exit
	push eax ; exitcode van main zit in eax
	call exit
	ret

filenamebuffer: db 'A@PRGS/CODE.C', 0
fileoutbuffer: db 'A@PRGS/TEGT.ASM', 0
fasm: db './compiler.bin', 0
dashm: db '-m', 0
ten: db '10', 0
_char_c dd fasm, filenamebuffer, fileoutbuffer, dashm, ten