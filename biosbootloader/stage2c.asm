bits 16

; glb bootdevice : unsigned char
section .data
	global	_bootdevice
_bootdevice:
; =
; RPN'ized expression: "0 "
; Expanded expression: "0 "
; Expression value: 0
	db	0

; glb getc : () char
section .text
	global	_getc
_getc:
	push	bp
	mov	bp, sp
	;sub	sp,          0
mov ax,0
int 0x16
L1:
	leave
	ret

; glb convert : (
; prm     num : unsigned
; prm     base : int
;     ) * char
section .text
	global	_convert
_convert:
	push	bp
	mov	bp, sp
	 sub	sp,          2
; loc     num : (@4) : unsigned
; loc     base : (@6) : int
; loc     Representation : L5 : [0u] char

section .data
L5:
; =
	db	"0123456789ABCDEF"
	times	1 db 0

section .text
; RPN'ized expression: "50 "
; Expanded expression: "50 "
; Expression value: 50
; loc     buffer : L6 : [50u] char

section .bss
L6:
	resb	50

section .text
; loc     ptr : (@-2) : * char
; RPN'ized expression: "ptr buffer 49 + *u &u = "
; Expanded expression: "(@-2) L6 49 + =(2) "
; Fused expression:    "+ L6 49 =(170) *(@-2) ax "
	mov	ax, L6
	add	ax, 49
	mov	[bp-2], ax
; RPN'ized expression: "ptr *u 0 = "
; Expanded expression: "(@-2) *(2) 0 =(-1) "
; Fused expression:    "*(2) (@-2) =(122) *ax 0 "
	mov	ax, [bp-2]
	mov	bx, ax
	mov	ax, 0
	mov	[bx], al
	cbw
; do
L7:
; {
; RPN'ized expression: "ptr -- *u Representation num base % + *u = "
; Expanded expression: "(@-2) --(2) L5 (@4) *(2) (@6) *(2) %u + *(-1) =(-1) "
; Fused expression:    "--(2) *(@-2) push-ax %u *(@4) *(@6) + L5 ax =(119) **sp *ax "
	dec	word [bp-2]
	mov	ax, [bp-2]
	push	ax
	mov	ax, [bp+4]
	mov	dx, 0
	div	word [bp+6]
	mov	ax, dx
	mov	cx, ax
	mov	ax, L5
	add	ax, cx
	mov	bx, ax
	mov	al, [bx]
	cbw
	pop	bx
	mov	[bx], al
	cbw
; RPN'ized expression: "num base /= "
; Expanded expression: "(@4) (@6) *(2) /=u(2) "
; Fused expression:    "/=u(170) *(@4) *(@6) "
	mov	ax, [bp+4]
	mov	dx, 0
	div	word [bp+6]
	mov	[bp+4], ax
; }
; while
; RPN'ized expression: "num 0 != "
; Expanded expression: "(@4) *(2) 0 != "
L8:
; Fused expression:    "!= *(@4) 0 IF "
	mov	ax, [bp+4]
	cmp	ax, 0
	jne	L7
L9:
; return
; RPN'ized expression: "ptr "
; Expanded expression: "(@-2) *(2) "
; Fused expression:    "*(2) (@-2)  "
	mov	ax, [bp-2]
L3:
	leave
	ret

; glb putc : (
; prm     a : char
;     ) void
section .text
	global	_putc
_putc:
	push	bp
	mov	bp, sp
	;sub	sp,          0
; loc     a : (@4) : char
mov ah,0x0e
int 0x10
L10:
	leave
	ret

; glb print_string : (
; prm     message : * char
;     ) void
section .text
	global	_print_string
_print_string:
	push	bp
	mov	bp, sp
	 sub	sp,          4
; loc     message : (@4) : * char
; loc     deze : (@-2) : char
; RPN'ized expression: "deze 0 = "
; Expanded expression: "(@-2) 0 =(-1) "
; Fused expression:    "=(170) *(@-2) 0 "
	mov	ax, 0
	mov	[bp-2], ax
; loc     pointer : (@-4) : int
; RPN'ized expression: "pointer 0 = "
; Expanded expression: "(@-4) 0 =(2) "
; Fused expression:    "=(170) *(@-4) 0 "
	mov	ax, 0
	mov	[bp-4], ax
; while
; RPN'ized expression: "1 "
; Expanded expression: "1 "
; Expression value: 1
L14:
; {
; RPN'ized expression: "deze message pointer + *u = "
; Expanded expression: "(@-2) (@4) *(2) (@-4) *(2) + *(-1) =(-1) "
; Fused expression:    "+ *(@4) *(@-4) =(119) *(@-2) *ax "
	mov	ax, [bp+4]
	add	ax, [bp-4]
	mov	bx, ax
	mov	al, [bx]
	cbw
	mov	[bp-2], al
	cbw
; RPN'ized expression: "pointer ++p "
; Expanded expression: "(@-4) ++p(2) "
; Fused expression:    "++p(2) *(@-4) "
	mov	ax, [bp-4]
	inc	word [bp-4]
; if
; RPN'ized expression: "deze 0 == "
; Expanded expression: "(@-2) *(-1) 0 == "
; Fused expression:    "== *(@-2) 0 IF! "
	mov	al, [bp-2]
	cbw
	cmp	ax, 0
	jne	L16
; {
; break
	jmp	L15
; }
L16:
; RPN'ized expression: "( deze putc ) "
; Expanded expression: " (@-2) *(-1)  putc ()2 "
; Fused expression:    "( *(-1) (@-2) , putc )2 "
	mov	al, [bp-2]
	cbw
	push	ax
	call	_putc
	sub	sp, -2
; }
	jmp	L14
L15:
L12:
	leave
	ret

; glb main : () void
section .text
	global	_main
_main:
	push	bp
	mov	bp, sp
	 sub	sp,          2

section .rodata
L20:
	db	10,13,"Hello world!",10,13,"Bootdevice: "
	times	1 db 0

section .text
; RPN'ized expression: "( L20 print_string ) "
; Expanded expression: " L20  print_string ()2 "
; Fused expression:    "( L20 , print_string )2 "
	push	L20
	call	_print_string
	sub	sp, -2
; RPN'ized expression: "( ( 16 , bootdevice convert ) print_string ) "
; Expanded expression: "  16  bootdevice *(1)  convert ()4  print_string ()2 "
; Fused expression:    "( ( 16 , *(1) bootdevice , convert )4 , print_string )2 "
	push	16
	mov	al, [_bootdevice]
	mov	ah, 0
	push	ax
	call	_convert
	sub	sp, -4
	push	ax
	call	_print_string
	sub	sp, -2

section .rodata
L21:
	db	10,13
	times	1 db 0

section .text
; RPN'ized expression: "( L21 print_string ) "
; Expanded expression: " L21  print_string ()2 "
; Fused expression:    "( L21 , print_string )2 "
	push	L21
	call	_print_string
	sub	sp, -2
; loc     buffer : (@-2) : * unsigned char
; loc     <something> : * unsigned char
; RPN'ized expression: "buffer 12288 (something22) = "
; Expanded expression: "(@-2) 12288 =(2) "
; Fused expression:    "=(170) *(@-2) 12288 "
	mov	ax, 12288
	mov	[bp-2], ax

section .rodata
L23:
	db	"Kernel starts with: "
	times	1 db 0

section .text
; RPN'ized expression: "( L23 print_string ) "
; Expanded expression: " L23  print_string ()2 "
; Fused expression:    "( L23 , print_string )2 "
	push	L23
	call	_print_string
	sub	sp, -2
; RPN'ized expression: "( buffer 1 + *u putc ) "
; Expanded expression: " (@-2) *(2) 1 + *(1)  putc ()2 "
; Fused expression:    "( + *(@-2) 1 *(1) ax , putc )2 "
	mov	ax, [bp-2]
	inc	ax
	mov	bx, ax
	mov	al, [bx]
	mov	ah, 0
	push	ax
	call	_putc
	sub	sp, -2
; RPN'ized expression: "( buffer 2 + *u putc ) "
; Expanded expression: " (@-2) *(2) 2 + *(1)  putc ()2 "
; Fused expression:    "( + *(@-2) 2 *(1) ax , putc )2 "
	mov	ax, [bp-2]
	add	ax, 2
	mov	bx, ax
	mov	al, [bx]
	mov	ah, 0
	push	ax
	call	_putc
	sub	sp, -2
; RPN'ized expression: "( buffer 3 + *u putc ) "
; Expanded expression: " (@-2) *(2) 3 + *(1)  putc ()2 "
; Fused expression:    "( + *(@-2) 3 *(1) ax , putc )2 "
	mov	ax, [bp-2]
	add	ax, 3
	mov	bx, ax
	mov	al, [bx]
	mov	ah, 0
	push	ax
	call	_putc
	sub	sp, -2
; for
L24:
	jmp	L24
L27:
; Fused expression:    "0  "
	mov	ax, 0
L18:
	leave
	ret



; Syntax/declaration table/stack:
; Bytes used: 210/15360


; Macro table:
; Macro __SMALLER_C__ = `0x0100`
; Macro __SMALLER_C_16__ = ``
; Macro __SMALLER_C_SCHAR__ = ``
; Bytes used: 63/5120


; Identifier table:
; Ident 
; Ident __floatsisf
; Ident __floatunsisf
; Ident __fixsfsi
; Ident __fixunssfsi
; Ident __addsf3
; Ident __subsf3
; Ident __negsf2
; Ident __mulsf3
; Ident __divsf3
; Ident __lesf2
; Ident __gesf2
; Ident bootdevice
; Ident getc
; Ident convert
; Ident num
; Ident base
; Ident putc
; Ident a
; Ident print_string
; Ident message
; Ident main
; Bytes used: 199/5632

; Next label number: 28
; Compilation succeeded.
