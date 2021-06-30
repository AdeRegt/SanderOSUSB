format	ELF

;
; headers
public exit
public gettimeofday
;public probeer
public malloc
public getenv
public fopen
public fclose
public time
public fread
public fwrite
public fseek
public ftell
public write
public free
extrn main 

section '.text' executable
;
; The linking puzzle for FASM for SOS
; By Alexandros de Regt
;

;
; Exists the process
; void exit(int status)
; status = 8
; ebp
exit:
    push ebp
    mov ebp,esp

    mov eax,0x01
    mov ebx, dword [ebp + 8]
    int 0x80

    pop ebp
    ret

;
; Get the time of the day
gettimeofday:
    pop eax
    pop eax
    mov eax,0x4E
    int 0x80
    ret

;
; the main routine
probeer:
    mov eax,0
    push eax
    push eax
    call main

;
; mallocate some memory
; void *malloc(unsigned long size)
; size = 8
; ebp
malloc:
    push ebp
    mov ebp,esp

    mov ebx, dword [ebp + 8]
    mov eax,0xC0
    int 0x80

    pop ebp
    ret

getenv:
    mov eax,0
    ret

fopen:
    pop eax
    pop eax
    mov eax,0
    ret

fclose:
    pop eax
    mov eax,0
    ret

time:
    pop eax
    mov eax,0
    ret

fread:
fwrite:
fseek:
ftell:
ret

; signed long write(int fd, const char *buf, unsigned long nbytes)
;   fd      =   8
;   buf     =  12
;   nbytes  =  16
; ebp
write:
    push ebp
    mov ebp,esp
    
    mov eax,0x04
    mov ebx, dword[ebp + 8]
    mov edx, dword[ebp + 16]
    mov ecx, dword[ebp + 12]
    int 0x80

    pop ebp
    ret

free:
ret