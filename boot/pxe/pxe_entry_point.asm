[bits 16]
[ORG 0x7C00]

;
; prepare env
; (this should not be nessecary so it is been removed)
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
%include "cstub.asm"
prepare32:
    mov ax, gdt_dataSeg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

    call _kernelpreloader
    cli
    hlt