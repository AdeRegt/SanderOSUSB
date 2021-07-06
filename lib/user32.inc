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
; SanderOSUSB Standard Libary
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
; int gettimeofday(unsigned long tv, unsigned long tz)
; tv = 8
; tz = 12
; ebp
gettimeofday:
    push ebp
    mov ebp,esp

    mov eax,0x4E
    int 0x80

    pop ebp
    ret

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

;
; Get env details
; const char *getenv(const char *name)
; name = 8
; ebp
getenv:
    push ebp
    mov ebp,esp

    mov eax,.emptyreturnvar

    pop ebp
    ret
    .emptyreturnvar db 0x00

;
; Opens a file
; unsigned long fopen(const char* filename, const char *mode)
; filename = 8
; mode = 12
; ebp
fopen:
    push ebp
    mov ebp,esp

    mov eax,0x05
    mov ebx,dword [ebp + 8]
    mov ecx,dword [ebp + 12]
    int 0x80

    pop ebp
    ret

;
; Closes a file
; int fclose(unsigned long stream)
; stream = 8
; ebp
fclose:
    push ebp
    mov ebp,esp

    mov eax,0x06
    mov ebx,dword [ebp + 8]
    int 0x80

    pop ebp
    ret

;
; Gets the time 
; unsigned long time(unsigned long t)
; t = 8
; ebp
time:
    push ebp
    mov ebp,esp

    mov eax,0x00

    pop ebp
    ret
;
; Reads some or the complete code of a file
; unsigned long fread(unsigned char *ptr, unsigned long size, unsigned long nmemb, unsigned long stream)
; ptr = 8
; size = 12
; nmemb = 16
; stream = 20
; ebp
fread:
    push ebp
    mov ebp,esp
    
    mov eax,0x03
    mov ebx, dword[ebp + 20]
    mov ecx, dword[ebp + 8]
    mov edx, dword[ebp + 16]
    int 0x80

    pop ebp
    ret

;
; Writes code to file
; unsigned long fwrite(const void *ptr, unsigned long size, unsigned long nmemb, unsigned long stream)
; ptr = 8
; size = 12
; nmemb = 16
; stream = 20
; ebp
fwrite:
    push ebp
    mov ebp,esp
    
    mov eax,0x04
    mov ebx, dword[ebp + 20]
    mov ecx, dword[ebp + 8]
    mov edx, dword[ebp + 12]
    int 0x80

    pop ebp
    ret
;
; Seeks a certain point in the filebuffer
; int fseek(unsigned long stream, long int offset, int whence)
; stream = 8
; offset = 12
; whence = 16
; ebp
fseek:
    push ebp
    mov ebp,esp

    mov ebx, dword [ebp + 8]
    mov ecx, dword [ebp + 12]
    mov edx, dword [ebp + 16]
    mov eax,0xC2
    int 0x80

    pop ebp
    ret

;
; Tells where the filepointer is
; long int ftell(unsigned long stream)
; stream = 8
; ebp
ftell:
    push ebp
    mov ebp,esp

    mov ebx, dword [ebp + 8]
    mov eax,0xC3
    int 0x80

    pop ebp
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

;
; Frees a resource
; void free(void *ptr)
; ptr = 8
; ebp
free:
    push ebp
    mov ebp,esp

    mov ebx, dword [ebp + 8]
    mov eax,0xC1
    int 0x80

    pop ebp
    ret