;
; Requires the SoS Development Kit
; 

	format	ELF
	public	_start
	extrn main
	
section '.text' executable
	
_start:
	push _char_c
	push 3
	call main
	ret

filenamebuffer: db 'A@PRGS/TEGT.C', 0
fileoutbuffer: db 'A@PRGS/TEGT.ASM', 0
fasm: db './compiler.bin', 0
_char_c dd fasm, filenamebuffer, fileoutbuffer