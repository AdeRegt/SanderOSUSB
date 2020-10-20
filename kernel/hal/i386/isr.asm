

; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'
global gdt_flush     ; Allows the C code to link to this
extern gp            ; Says that '_gp' is in another file
gdt_flush:
    lgdt [gp]        ; Load the GDT with our '_gp' which is a special pointer
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
    ret               ; Returns back to the C code!
    
; Loads the IDT defined in '_idtp' into the processor.
; This is declared in C as 'extern void idt_load();'
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret
    
global vbirq
extern irq_vb
vbirq:
    call irq_vb
    iret
    
global ideirq
extern irq_ide
ideirq:
    call irq_ide
    iret
    
global serialirq
extern irq_serial
serialirq:
    call irq_serial
    iret
    
global keyboardirq
extern irq_keyboard
keyboardirq:
    call irq_keyboard
    iret
    
global keywait
extern keyword
keywait:
mov al,0
mov [keyword],al
.again:
mov al,[keyword]
cmp al,0
je .again
push ax
mov al,0
mov [keyword],al
pop ax
ret

global mouseirq
extern irq_mouse
mouseirq:
	
	call irq_mouse
	
    iret
    

global uhciirq
extern irq_uhci
uhciirq:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp
    push eax
    mov eax, irq_uhci
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
    

global rtl8169irq
extern irq_rtl8169
rtl8169irq:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp
    push eax
    mov eax, irq_rtl8169
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
    

global sndirq
extern irq_snd
sndirq:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp
    push eax
    mov eax, irq_snd
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
    

global ehciirq
extern irq_ehci
ehciirq:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp
    push eax
    mov eax, irq_ehci
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

global xhciirq
extern irq_xhci
xhciirq:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp
    push eax
    mov eax, irq_xhci
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
    

global timerirq
extern irq_multitasking
timerirq:
    pusha
	push ds
	push es
	push fs
	push gs
	mov eax, esp   ; Push us the stack
	push eax
	mov eax, irq_multitasking
	call eax       ; A special call, preserves the 'eip' register
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	iret
    

global irq_common_stub
extern irq_handler
irq_common_stub:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp
    push eax
    mov eax, irq_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
    
global isr_common_stub
extern fault_handler    
isr_common_stub:
	push byte 0
	push byte 0
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, esp   ; Push us the stack
    push eax
    mov eax, fault_handler
    call eax       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
    
    
global isr_special_stub
extern special_handler
isr_special_stub:
	pusha
	push ds
	push es
	push fs
	push gs
	mov eax, esp   ; Push us the stack
	push eax
	mov eax, special_handler
	call eax       ; A special call, preserves the 'eip' register
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
