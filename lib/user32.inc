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
public ungetc
public getc
public realpath
public realloc

public getnetworkpackage
public sendnetworkpackage
public initialisenetworkpackage

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

;
; Get first available character from stream
; int getc(FILE *stream)
; stream = 8
; ebp
getc:
    push ebp 
    mov ebp,esp 

    mov ebx, dword [ebp +8]
    mov eax,0xC5
    int 0x80

    pop ebp
    ret

;
; Put first available character to stream
; int ungetc(int chartoadd, FILE *stream)
; chartoadd = 8
; stream = 12
; ebp
ungetc:
    push ebp
    mov ebp,esp 

    mov ebx, dword [ebp + 12]
    mov ecx, dword [ebp + 8]
    mov eax,0xC4
    int 0x80 

    pop ebp 
    ret

;
; Gets full path from relative path
; char *realpath(const char * file_name,char * resolved_name)
; file_name = 8
; resolved_name = 12
; ebp
realpath:
    push ebp
    mov ebp,esp 

    mov ebx, dword [ebp + 8]
    mov ecx, dword [ebp + 12]
    mov eax,0xC6
    int 0x80 

    pop ebp 
    ret

;
; Reallocates block
; void *realloc(void *ptr, size_t size)
; ptr = 8
; size = 12
; ebp
realloc:
    push ebp
    mov ebp,esp 

    mov ebx, dword [ebp + 8]
    mov ecx, dword [ebp + 12]
    mov eax,0xC7
    int 0x80 

    pop ebp 
    ret

;
; Gets a package from the network
; void *getethetnerpackage()
; ebp
getnetworkpackage:
    push ebp
    mov ebp,esp 

    mov eax,0xC8
    int 0x80 

    pop ebp
    ret

;
; Sends a package to the network
; void sendnetworkpackage(int type,int size,unsigned char *to,void *where,int port);
; type    =   8
; size    =  12
; to      =  16
; where   =  20
; port   =  24
; ebp
sendnetworkpackage:
    push ebp 
    mov ebp,esp 

    mov eax,0xC9
    mov ebx,dword [ebp+8]
    mov ecx,dword [ebp+12]
    mov edx,dword [ebp+16]
    mov esi,dword [ebp+20]
    mov edi,dword [ebp+24]
    int 0x80 

    pop ebp
    ret

;
; Initialises a networksocket
; char initialisenetworkpackage(int type,int is_ip, unsigned char *to, int function, int port);
; type      = 8
; is_ip     = 12
; to        = 16
; function  = 20
; port      = 24
; ebp
initialisenetworkpackage:
    push ebp 
    mov ebp,esp 

    mov eax,0xCA
    mov ebx,dword [ebp+8]
    mov ecx,dword [ebp+12]
    mov edx,dword [ebp+16]
    mov esi,dword [ebp+20]
    mov edi,dword [ebp+24]
    int 0x80 

    pop ebp
    ret
