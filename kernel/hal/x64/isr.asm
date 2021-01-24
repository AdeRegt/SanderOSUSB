[bits 64]





global getCSValue:
getCSValue:
mov rax,0
mov rax,cs 
ret

global isr_common_stub
isr_common_stub:
push rax
mov ax,0x20
out 0x20,ax
mov ax,0x20
out 0xA0,ax
pop rax
iretq