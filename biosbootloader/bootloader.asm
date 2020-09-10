; 
; This is the bootloader for SanderOSUSB
; The CPU is currently 16 bit
; We will load our second stage as first task
;
; This bootloader is made for a FAT12 filesystem on a floppy/usb
; Some parts of the bootloader has been taken from:
; https://github.com/mig-hub/mikeOS/blob/master/source/bootload/bootload.asm
;

BITS 16

;
; First we need to make the device discoverable by a OS as a FAT16 disk
; This we do with a Disk Descriptor Table
; But code is already executed here so we need to jump over it
;

jmp short bootloader_start
nop 

OEMLabel			db "SANDEROS"		; Disk label
BytesPerSector		dw 0x200			; Bytes per sector
SectorsPerCluster	db 0x10				; Sectors per cluster
ReservedForBoot		dw 0x10				; Reserved sectors for boot record
NumberOfFats		db 2				; Number of copies of the FAT
RootDirEntries		dw 0x200			; Number of entries in root dir;
										; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors		dw 0				; Number of logical sectors
MediumByte			db 0xF8				; Medium descriptor byte
SectorsPerFat		dw 1				; Sectors per FAT
SectorsPerTrack		dw 0x3E				; Sectors per track (36/cylinder)
Sides				dw 0x10				; Number of sides/heads
HiddenSectors		dd 0				; Number of hidden sectors
LargeSectors		dd 0				; Number of LBA sectors
DriveNo				dw 0x80				; Drive No: 0
Signature			db 0x29				; Drive signature: 41 for floppy
VolumeID			dd 00000000h		; Volume ID: any number
VolumeLabel			db "SANDEROSUSB"	; Volume Label: any 11 chars
FileSystem			db "FAT12   "		; File system type: don't change!
IsSOSBootDisk		db 0xCD
SectorOfBootDIR		dw 18
SectorOfData		dw 0x3F
loadloc				dw 0x2000
loadcount			dw 0x03

; 00002420  53 54 41 47 45 32 20 20  42 49 4e 20 00 9a 36 74  |STAGE2  BIN ..6t|
; 00002430  25 51 25 51 00 00 36 74  25 51 03 00 9d 01 00 00  |%Q%Q..6t%Q......|

; 00002460  4b 45 52 4e 45 4c 20 20  42 49 4e 20 00 9f 36 74  |KERNEL  BIN ..6t|
; 00002470  25 51 25 51 00 00 36 74  25 51 04 00 fc eb 01 00  |%Q%Q..6t%Q......|

; stage2 on disk : 0x42   kernel on disk : 0x52

;
; Lets start!
;

bootloader_start:
	; First, setup the system
	mov ax, 07C0h						; Set up 4K of stack space above buffer
	add ax, 544							; 8k buffer = 512 paragraphs + 32 paragraphs (loader)
	cli									; Disable interrupts while changing stack
	mov ss, ax
	mov sp, 4096
	sti									; Restore interrupts

	mov ax, 07C0h						; Set data segment to where we're loaded
	mov ds, ax

	mov [bootdevice] , dl				; save bootdevice for later

	call reset_bootdevice 				; reset bootdevice to a known state

	mov ah,0x08
	int 0x13
	and cx,0x3F
	mov word [SectorsPerTrack], cx
	movzx dx, dh
	add dx,1
	mov word [Sides],dx

	mov ax,word [SectorOfBootDIR] 		; sector of the rootdirectory
	call l2hts							; convert lba to chs

	mov si,buffer
	mov bx,ds
	mov es,bx
	mov bx,si

	mov al,2							; read 14 sectors
	call read_sector					; read sector from disk

	call loadFile

	mov si,filename
	mov byte [si] , 'K'
	inc si
	mov byte [si] , 'E'
	inc si
	mov byte [si] , 'R'
	inc si
	mov byte [si] , 'N'
	inc si
	mov byte [si] , 'E'
	inc si
	mov byte [si] , 'L'
	mov si,0x3000
	mov word [loadloc],si
	mov al, 0xE0;0xEF
	mov byte[loadcount], al
	
	call loadFile

	mov si,filefound
	call print_string

	mov si,0x1000
	mov byte [si],al
	jmp 2000h:0000h

	cli
	hlt

	;
	; Time to look for the file
	;

loadFile:

	mov ax,ds
	mov es,ax

	mov di,buffer
	mov cx,224
	mov ax,0

next_fs_entry:
	xchg cx,dx
	mov si,filename
	mov cx,11
	rep cmpsb
	je fs_entry_found
	add ax,32
	mov di,buffer
	add di,ax
	xchg dx,cx
	loop next_fs_entry

	mov si,filenotfound
	call print_string
	cli
	hlt

; stage2 on disk : 0x42[3]   kernel on disk : 0x52
fs_entry_found:
	mov al,0
	mov byte[over],al
	mov cx,0x10
	mov ax, word [es:di+0xF]
	inc ax
	mul cx
	add ax,2
	mov word [cluster], ax
	mov cx,word[loadcount]
nogmaals:
	push cx
	mov ax, word [cluster]
	call l2hts
	mov ax, word [loadloc]
	mov es,ax
	mov bx,0
	mov al,1
	call read_sector

	mov ax, word [cluster]
	add ax,1
	mov word [cluster],ax

	mov ax, word [loadloc]
	add ax,0x200
	mov word [loadloc],ax

	pop cx
	loop nogmaals
	overslaan:
	ret

over dw 0
;
; Function to reset bootdevice to a known state
; IN = none
; OUT = none
;

reset_bootdevice:
	pusha
	mov ax,0x00
	mov dl,byte [bootdevice]
	int 0x13
	jc error
	popa
	ret

;
; Calculate head, track and sector settings for int 13h
; IN: logical sector in AX
; OUT: correct registers for int 13h
;

l2hts:
	push bx
	push ax

	mov bx, ax			; Save logical sector

	mov dx, 0			; First the sector
	div word [SectorsPerTrack]
	add dl, 01h			; Physical sectors start at 1
	mov cl, dl			; Sectors belong in CL for int 13h
	mov ax, bx

	mov dx, 0			; Now calculate the head
	div word [SectorsPerTrack]
	mov dx, 0
	div word [Sides]
	mov dh, dl			; Head/side
	mov ch, al			; Track

	pop ax
	pop bx

	mov dl, byte [bootdevice]		; Set correct device

	ret

;
; Read sector
; IN: al sectorcount
;
read_sector:
	pusha

	mov ah,2

	stc
	int 0x13
	jc error
	popa
	ret

;
; Prints a string on the screen
; IN: SI the string to print
;
print_string:
	pusha
	mov ah,0x0E
	.again:
		lodsb
		cmp al,0x00
		je .done
		int 0x10
		jmp short .again
	.done:
	popa
	ret

error:
	mov si,errormsg
	call print_string
	cli
	hlt

;
; Space for variables
;

bootdevice db 0x00
cluster dw 0x0000
filename db "STAGE2  BIN",0x00
filefound db "Stage 2 found ",0x00
filenotfound db "Stage 2 not found",0x00
errormsg db "ERROR",0x00

;
; End bootloader like all does
;

times 510-($-$$) db 0
dw 0AA55h

buffer: