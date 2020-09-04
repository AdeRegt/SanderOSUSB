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
BytesPerSector		dw 512				; Bytes per sector
SectorsPerCluster	db 1				; Sectors per cluster
ReservedForBoot		dw 1				; Reserved sectors for boot record
NumberOfFats		db 2				; Number of copies of the FAT
RootDirEntries		dw 224				; Number of entries in root dir
										; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors		dw 2880				; Number of logical sectors
MediumByte			db 0F0h				; Medium descriptor byte
SectorsPerFat		dw 9				; Sectors per FAT
SectorsPerTrack		dw 18				; Sectors per track (36/cylinder)
Sides				dw 2				; Number of sides/heads
HiddenSectors		dd 0				; Number of hidden sectors
LargeSectors		dd 0				; Number of LBA sectors
DriveNo				dw 0				; Drive No: 0
Signature			db 41				; Drive signature: 41 for floppy
VolumeID			dd 00000000h		; Volume ID: any number
VolumeLabel			db "SANDEROSUSB"	; Volume Label: any 11 chars
FileSystem			db "FAT12   "		; File system type: don't change!

;
; Lets start!
;

bootloader_start:
	; First, setup the system
	mov ax, 07C0h			; Set up 4K of stack space above buffer
	add ax, 544				; 8k buffer = 512 paragraphs + 32 paragraphs (loader)
	cli						; Disable interrupts while changing stack
	mov ss, ax
	mov sp, 4096
	sti						; Restore interrupts

	mov ax, 07C0h			; Set data segment to where we're loaded
	mov ds, ax

	mov [bootdevice] , dl	; save bootdevice for later

	call reset_bootdevice 	; reset bootdevice to a known state

	mov ax,19 				; sector of the rootdirectory
	call l2hts				; convert lba to chs

	mov si,buffer
	mov bx,ds
	mov es,bx
	mov bx,si

	mov al,14				; read 14 sectors
	call read_sector		; read sector from disk

	;
	; Time to look for the file
	;

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

fs_entry_found:
	mov ax, word [es:di+0x0F]
	mov word [cluster], ax
	mov ax, word [cluster]
	add ax,31
	call l2hts
	mov ax,0x2000
	mov es,ax
	mov bx,0
	mov al,0xA
	call read_sector

	mov si,filefound
	call print_string

	jmp 2000h:0000h

	cli
	hlt


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

	int 0x13
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

;
; Space for variables
;

bootdevice db 0x00
cluster dw 0x0000
filename db "STAGE2  BIN"
filefound db "Stage 2 found ",0x00
filenotfound db "Stage 2 not found",0x00

;
; End bootloader like all does
;

times 510-($-$$) db 0
dw 0AA55h

buffer: