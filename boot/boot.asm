; Custom Bootloader v1

; Let's get the registers and the stuck out of the way for now. 
; In order to update the CS register we need to do a far jmp
; but first we need to create space to make sure that the BPB 
; doesn't overwrite our code.  

ORG 0
BITS 16

; Filling up the bytes where the bios *might* write the BPB to

jmp short step1
nop

times 33 db 0

step1:
    jmp 0x7c0:step2

step2: ; Set up the segment registers to not point to 0x0, except for SS
    cli
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    sti

mov [bootdev], dl ; Save the boot device code in case it's required later.


done: cli
jmp hlt

; Useful routines

print:
    pusha
    mov si, ax
    mov bx, 0
.loop:
    lodsb
    cmp al, 0
    je .done
    call print_char
    jmp .loop
.done:
    popa
    ret    

print_char:
    mov ah, 0xe
    int 0x10
    ret



hlt: cli
jmp hlt

bootdev: db 0, 0
message: db "Correct", 0


times 510 - ($ - $$) db 0
dw 0xAA55


db "Test", 0