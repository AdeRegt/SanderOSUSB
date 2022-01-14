;
; Requires the SoS Development Kit
; 

	format	ELF
	public	_start
	extrn main
	extrn exit


section .bss
align 16
; stack_bottom:
; resb 16384 ; 16 KiB
; stack_top:
	
section '.text' executable
	
_start:
    ; cli
    ; mov esp, stack_top
	; xor ebp, ebp
    ; sti

	push _char_c
	push 1
	call main
	; exit
	push eax ; exitcode van main zit in eax
	call exit
	ret

fasm: db './compiler.bin', 0
_char_c dd fasm,0