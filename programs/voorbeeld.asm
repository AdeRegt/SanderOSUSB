[bits 32]
[org 0x2000]
main:

; call system to print hello world
mov eax,4
mov ebx,1
mov ecx,message
mov edx,11
int 0x80



; call system to stop
mov eax,1
mov ebx,0
mov ecx,0
mov edx,0
int 0x80

cli
hlt

message db "Hello world",0x00