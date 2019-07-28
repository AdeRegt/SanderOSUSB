[bits 32]
global _start
extern main
_start:
; call main
call main

; call system to stop
mov eax,1
int 0x80
