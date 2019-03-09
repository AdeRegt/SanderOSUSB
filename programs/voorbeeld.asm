bits 32
org 0x2000

mov esi,0xb8000
mov al,'F'
mov byte [esi],al

mov eax,0xCD
int 0x80
cmp eax,0xff
je skp
mov esi,0xb8000
mov al,'X'
mov byte [esi],al

skp:
cli
hlt
