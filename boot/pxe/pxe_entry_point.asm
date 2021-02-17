[bits 16]
[ORG 0x7C00]

;
; prepare env
; (this should not be nessecary so it is been removed)
codestart:
    mov ax, 0
    mov ds, ax

    ; enable A20 line
    mov ax,2401h
    int 15h

    ; disable interrupts
    cli
    ; enable gdt
    lgdt [gdt_pointer]
    ; set cpu to 32bits
    mov eax, cr0
    or eax,1
    mov cr0, eax
    ; jump to32bits
    jmp gdt_codeSeg:prepare32

gdt_pointer:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; format
; 1th double word (32)
;   00-15       first 16 bits of the segment limiter
;   16-31       first 16 bits of the base address
; 2nd double word (32)
;   00-07       Bits 16-23 in the base address
;   08-12       Segment type and attributes
;   13-14       0 = Highest privilege (OS), 3 = Lowest privilege (User applications)
;   15          Set to 1 if segment is present
;   16-19       Bits 16-19 in the segment limiter
;   20-22       Different attributes, depending on the segment type
;   23          Used together with the limiter, to determine the size of the segment
;   24-31       The last 24-31 bits in the base address
gdt_start:
    ; null-segment
    gdt_null:
        dd 0
        dd 0
    ; code-segment
    gdt_code:
        dw 0FFFFh
        dw 0
        db 0
        db 0x9a
        db 0xcf
        db 0
    ; data-segment
    gdt_data:
        dw 0FFFFh
        dw 0
        db 0
        db 0x92
        db 0xcf
        db 0
gdt_end:

gdt_codeSeg equ gdt_code - gdt_start
gdt_dataSeg equ gdt_data - gdt_start

[bits 32]
prepare32:
    mov ax, gdt_dataSeg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

;
; Tell the user that we are alive by printing an Y on the screen

    mov esi,0xb8000
    mov bl,byte 'Y'
    mov byte [esi],bl
    inc esi
    mov bl,0xCD
    mov byte [esi],bl

;
; Look for the ELF header

    mov esi,ende
.findTemplate:
    mov al, byte[esi]
    mov bl, 0x7F
    cmp al,bl
    je .foundA
    add esi,1
    jmp .findTemplate
.foundA:
    inc esi
    mov al, byte[esi]
    mov bl, 0x45
    cmp al,bl
    je .foundB
    inc esi
    jmp .findTemplate
.foundB:
    inc esi
    mov al, byte[esi]
    mov bl, 0x4c
    cmp al,bl
    je .yat
    inc esi
    jmp .findTemplate

;
; We found the header! 
.yat:
    dec esi
    dec esi
    mov dword[offset_elf_header],esi

;
; Lets get the section table address
    mov esi,dword [offset_elf_header]
    add esi,32
    mov eax,dword [esi] 
    mov dword[offset_elf_sections], eax

;
; Lets get the section table count
    mov esi,dword [offset_elf_header]
    add esi,48
    mov ax,word [esi] 
    mov word[offset_elf_sections_count], ax

;
; Lets get the entrypoint
    mov esi,dword [offset_elf_header]
    add esi,24
    mov eax,dword [esi] 
    mov dword[entrypoint], eax

;
; Resolve all sections
.resolveSection:
    ; how many times left?
    mov ecx,0
    mov cx, word [offset_elf_sections_count]
    mov ax,0
    cmp cx,ax
    je .ok
    ; calculate address of section
    mov eax,0
    mov eax,ecx
    dec eax ; always -1
    mov ebx, 40
    mul ebx ; eax = eax*ebx
    mov ebx,0
    mov bx, word [offset_elf_header]
    add eax, ebx
    mov ebx, 0
    mov bx, word [offset_elf_sections]
    add eax, ebx
    ; eax should contain the address to go to the address
    mov dword[tmp_section], eax
    ; check address
    mov esi, dword[tmp_section]
    add esi,11 
    mov eax,dword[esi]
    mov ebx,0
    cmp eax,ebx
    je .notinterestingsegment
    cmp eax,0x0011f000
    je .T0
    ; address is not null, do we need to copy it?
    mov eax, dword[tmp_section]
    mov ecx,eax
    mov esi,ecx
    add esi,4 
    mov eax,dword[esi]
    mov ebx,1
    cmp eax,ebx
    jne .notinterestingsegment
.T0:
    mov esi,0xb8000
    mov bl,byte 'T'
    mov byte [esi],bl
    inc esi
    mov bl,0xCD
    mov byte [esi],bl
    cli
    hlt
    ; decrease with one
.notinterestingsegment:
    mov cx, word [offset_elf_sections_count]
    dec cx
    mov word [offset_elf_sections_count], cx
    jmp .resolveSection
.notok:
    jmp .notok

;
; Tell the world we are all done!
.ok:
    mov esi,0xb8000
    mov bl,byte 'X'
    mov byte [esi],bl
    inc esi
    mov bl,0xCD
    mov byte [esi],bl
    hlt

    db 0xcd
offset_elf_header dd 0
    db 0xcd
offset_elf_sections dd 0
    db 0xcd
entrypoint dd 0
    db 0xcd
tmp_section dd 0
    db 0xcd
offset_elf_sections_count dw 0
    db 0xcd

ende;
    db 0x00
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0xcd
    db 0x00