[bits 16]
[ORG 0x7C00]

;
; prepare env
; (this should not be nessecary so it is been removed)
mov ax, 0
mov ds, ax

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

    call enable_A20

    ;
    ; Mijn eigen wanhopige poging...

    ; Klopt de elfsignatuur?
    ; deze moet zijn: 
    ;   0x7f
    ;   E
    ;   L
    ;   F
    mov esi,begin_van_het_einde
    inc esi
    mov al,byte [esi]
    mov bl,'E'
    cmp al,bl
    jne not_oke
    inc esi
    mov al,byte [esi]
    mov bl,'L'
    cmp al,bl
    jne not_oke
    inc esi
    mov al,byte [esi]
    mov bl,'F'
    cmp al,bl
    jne not_oke
    inc esi

    ;
    ; vanaf dit moment weten we dat de elfsignatuur klopt

    ;
    ; nu opzoeken waar we de e_shoff kunnen vinden
    ; als het goed is, is dit getal + 0x20 offset
    mov esi,begin_van_het_einde
    add esi,0x20
    mov eax, dword[esi]
    add eax,begin_van_het_einde
    mov dword[e_shoff],eax

    ;
    ; nu opzoeken waar de e_shnum is
    ; als het goed is, is dit getal + 0x30 offset
    mov esi,begin_van_het_einde
    add esi,0x30
    mov ax, word[esi]
    mov word[sh_size],ax

    ;
    ; nu opzoeken waar de e_entry is
    ; als het goed is, is dit getal + 0x30 offset
    mov esi,begin_van_het_einde
    add esi,0x18
    mov eax, dword[esi]
    mov dword[e_entry],eax

    ;
    ; nu over de secties lopen
    ; er zouden 16 secties zijn met een address
    mov esi,dword [e_shoff]
    mov cx,word [sh_size]
check_section_again:
    push esi 
    push cx
    ; alle velden vullen en ondertussen controleren
    mov eax,dword [esi]
    mov dword [tmp_sh_name], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_type], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_flags], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_addr], eax
    add esi,4
    mov eax,dword [esi]
    add eax,begin_van_het_einde
    mov dword [tmp_sh_offset], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_size], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_link], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_info], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_addralign], eax
    add esi,4
    mov eax,dword [esi]
    mov dword [tmp_sh_entsize], eax
    add esi,4
    ; hebben we voldoende middelen?
    ; addr moet niet 0 zijn
    mov eax,dword [tmp_sh_addr]
    cmp eax,0
    je not_interested_anymore_in_section
    ; type moet 1 zijn
    mov eax,dword [tmp_sh_type]
    cmp eax,1
    jne not_interested_anymore_in_section
    ; alles lijkt ok
    ; we hebben een sectie die gekopieerd kan worden
    ; tmp_sh_offset : van
    ; tmp_sh_addr : naar
    ; tmp_sh_size : size
    mov esi, dword [tmp_sh_offset]
    mov edi, dword [tmp_sh_addr]
    mov ecx, dword [tmp_sh_size]
copy_location_of_code:
    mov al, byte[esi]
    mov byte[edi], al 
    inc esi
    inc edi
    loop copy_location_of_code
    mov ecx,0
not_interested_anymore_in_section:
    pop cx
    pop esi 
    add esi,40
    dec cx
    cmp cx,0
    jne check_section_again

    ; lets call the kernel!!
    mov eax, dword [e_entry]
    call eax
    ; just to make sure it doesnt do bad things
    cli
    hlt
not_oke:
    mov esi,0xb8000
    mov al,byte 'Y'
    mov byte [esi],al 
    inc esi 
    mov al,0x60
    mov byte [esi],al 
    cli
    hlt

; A20 function from: https://wiki.osdev.org/A20_Line
enable_A20:
        cli
 
        call    a20wait
        mov     al,0xAD
        out     0x64,al
 
        call    a20wait
        mov     al,0xD0
        out     0x64,al
 
        call    a20wait2
        in      al,0x60
        push    eax
 
        call    a20wait
        mov     al,0xD1
        out     0x64,al
 
        call    a20wait
        pop     eax
        or      al,2
        out     0x60,al
 
        call    a20wait
        mov     al,0xAE
        out     0x64,al
 
        call    a20wait
        sti
        ret
 
a20wait:
        in      al,0x64
        test    al,2
        jnz     a20wait
        ret
 
 
a20wait2:
        in      al,0x64
        test    al,1
        jz      a20wait2
        ret

; Declaraties van variables
e_shoff dd 0
sh_size dw 0
e_entry dd 0
; Temp sectie vullers
tmp_sh_name dd 0
tmp_sh_type dd 0
tmp_sh_flags dd 0
tmp_sh_addr dd 0
tmp_sh_offset dd 0
tmp_sh_size dd 0
tmp_sh_link dd 0
tmp_sh_info dd 0
tmp_sh_addralign dd 0
tmp_sh_entsize dd 0
begin_van_het_einde: