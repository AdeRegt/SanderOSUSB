bits 32

; glb Elf32_Word : unsigned
; glb Elf32_Addr : unsigned
; glb Elf32_Off : unsigned
; glb Elf32_Sword : unsigned
; glb Elf32_Half : unsigned short
; RPN'ized expression: "16 "
; Expanded expression: "16 "
; Expression value: 16
; glb ELFHEADER : struct <something>
; glb ELFSECTION : struct <something>
; glb x : unsigned char
section .data
	global	_x
_x:
; =
; RPN'ized expression: "0 "
; Expanded expression: "0 "
; Expression value: 0
	db	0

; glb y : unsigned char
section .data
	global	_y
_y:
; =
; RPN'ized expression: "0 "
; Expanded expression: "0 "
; Expression value: 0
	db	0

; glb putch : (
; prm     d : char
;     ) void
section .text
	global	_putch
_putch:
	push	ebp
	mov	ebp, esp
	 sub	esp,          4
; loc     d : (@8) : char
; if
; RPN'ized expression: "d 10 == "
; Expanded expression: "(@8) *(-1) 10 == "
; Fused expression:    "== *(@8) 10 IF! "
	mov	al, [ebp+8]
	movsx	eax, al
	cmp	eax, 10
	jne	L3
; {
; RPN'ized expression: "x 0 = "
; Expanded expression: "x 0 =(1) "
; Fused expression:    "=(156) *x 0 "
	mov	eax, 0
	mov	[_x], al
	movzx	eax, al
; RPN'ized expression: "y ++p "
; Expanded expression: "y ++p(1) "
; Fused expression:    "++p(1) *y "
	mov	al, [_y]
	movzx	eax, al
	inc	byte [_y]
; return
	jmp	L1
; }
L3:
; loc     ptr : (@-4) : int
; RPN'ized expression: "ptr y 80 * x + 2 * = "
; Expanded expression: "(@-4) y *(1) 80 * x *(1) + 2 * =(4) "
; Fused expression:    "* *y 80 + ax *x * ax 2 =(204) *(@-4) ax "
	mov	al, [_y]
	movzx	eax, al
	imul	eax, eax, 80
	movzx	ecx, byte [_x]
	add	eax, ecx
	imul	eax, eax, 2
	mov	[ebp-4], eax
; loc     <something> : * unsigned char
; RPN'ized expression: "753664 (something5) ptr + *u d = "
; Expanded expression: "753664 (@-4) *(4) + (@8) *(-1) =(1) "
; Fused expression:    "+ 753664 *(@-4) =(151) *ax *(@8) "
	mov	eax, 753664
	add	eax, [ebp-4]
	mov	ebx, eax
	mov	al, [ebp+8]
	movsx	eax, al
	mov	[ebx], al
	movzx	eax, al
; loc     <something> : * unsigned char
; RPN'ized expression: "753664 (something6) ptr 1 + + *u 96 = "
; Expanded expression: "753664 (@-4) *(4) 1 + + 96 =(1) "
; Fused expression:    "+ *(@-4) 1 + 753664 ax =(156) *ax 96 "
	mov	eax, [ebp-4]
	inc	eax
	mov	ecx, eax
	mov	eax, 753664
	add	eax, ecx
	mov	ebx, eax
	mov	eax, 96
	mov	[ebx], al
	movzx	eax, al
; RPN'ized expression: "x ++p "
; Expanded expression: "x ++p(1) "
; Fused expression:    "++p(1) *x "
	mov	al, [_x]
	movzx	eax, al
	inc	byte [_x]
; if
; RPN'ized expression: "x 80 == "
; Expanded expression: "x *(1) 80 == "
; Fused expression:    "== *x 80 IF! "
	mov	al, [_x]
	movzx	eax, al
	cmp	eax, 80
	jne	L7
; {
; RPN'ized expression: "x 0 = "
; Expanded expression: "x 0 =(1) "
; Fused expression:    "=(156) *x 0 "
	mov	eax, 0
	mov	[_x], al
	movzx	eax, al
; RPN'ized expression: "y ++p "
; Expanded expression: "y ++p(1) "
; Fused expression:    "++p(1) *y "
	mov	al, [_y]
	movzx	eax, al
	inc	byte [_y]
; }
L7:
L1:
	leave
	ret

; glb printstring : (
; prm     message : * char
;     ) void
section .text
	global	_printstring
_printstring:
	push	ebp
	mov	ebp, esp
	 sub	esp,          8
; loc     message : (@8) : * char
; loc     i : (@-4) : int
; RPN'ized expression: "i 0 = "
; Expanded expression: "(@-4) 0 =(4) "
; Fused expression:    "=(204) *(@-4) 0 "
	mov	eax, 0
	mov	[ebp-4], eax
; while
; RPN'ized expression: "1 "
; Expanded expression: "1 "
; Expression value: 1
L11:
; {
; loc         deze : (@-8) : char
; RPN'ized expression: "deze message i ++p + *u = "
; Expanded expression: "(@-8) (@8) *(4) (@-4) ++p(4) + *(-1) =(-1) "
; Fused expression:    "++p(4) *(@-4) + *(@8) ax =(199) *(@-8) *ax "
	mov	eax, [ebp-4]
	inc	dword [ebp-4]
	mov	ecx, eax
	mov	eax, [ebp+8]
	add	eax, ecx
	mov	ebx, eax
	mov	al, [ebx]
	movsx	eax, al
	mov	[ebp-8], eax
; if
; RPN'ized expression: "deze 0 == "
; Expanded expression: "(@-8) *(-1) 0 == "
; Fused expression:    "== *(@-8) 0 IF! "
	mov	al, [ebp-8]
	movsx	eax, al
	cmp	eax, 0
	jne	L13
; {
; break
	jmp	L12
; }
L13:
; RPN'ized expression: "( deze putch ) "
; Expanded expression: " (@-8) *(-1)  putch ()4 "
; Fused expression:    "( *(-1) (@-8) , putch )4 "
	mov	al, [ebp-8]
	movsx	eax, al
	push	eax
	call	_putch
	sub	esp, -4
; }
	jmp	L11
L12:
L9:
	leave
	ret

; glb kernelpreloader : () void
section .text
	global	_kernelpreloader
_kernelpreloader:
	push	ebp
	mov	ebp, esp
	 sub	esp,         60
; loc     elfaddr : (@-4) : unsigned
; RPN'ized expression: "elfaddr 31744 = "
; Expanded expression: "(@-4) 31744 =(4) "
; Fused expression:    "=(204) *(@-4) 31744 "
	mov	eax, 31744
	mov	[ebp-4], eax
; while
; RPN'ized expression: "1 "
; Expanded expression: "1 "
; Expression value: 1
L17:
; {
; loc         A : (@-8) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "A elfaddr 0 + (something19) 0 + *u = "
; Expanded expression: "(@-8) (@-4) *(4) 0 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 0 =(201) *(@-8) *ax "
	mov	eax, [ebp-4]
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-8], eax
; loc         B : (@-12) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "B elfaddr 1 + (something20) 0 + *u = "
; Expanded expression: "(@-12) (@-4) *(4) 1 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 1 =(201) *(@-12) *ax "
	mov	eax, [ebp-4]
	inc	eax
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-12], eax
; loc         C : (@-16) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "C elfaddr 2 + (something21) 0 + *u = "
; Expanded expression: "(@-16) (@-4) *(4) 2 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 2 =(201) *(@-16) *ax "
	mov	eax, [ebp-4]
	add	eax, 2
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-16], eax
; loc         D : (@-20) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "D elfaddr 3 + (something22) 0 + *u = "
; Expanded expression: "(@-20) (@-4) *(4) 3 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 3 =(201) *(@-20) *ax "
	mov	eax, [ebp-4]
	add	eax, 3
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-20], eax
; if
; RPN'ized expression: "A 127 == B 69 == && C 76 == && D 70 == && "
; Expanded expression: "(@-8) *(1) 127 == [sh&&->27] (@-12) *(1) 69 == &&[27] _Bool [sh&&->26] (@-16) *(1) 76 == &&[26] _Bool [sh&&->25] (@-20) *(1) 70 == &&[25] "
; Fused expression:    "== *(@-8) 127 [sh&&->27] == *(@-12) 69 &&[27] _Bool [sh&&->26] == *(@-16) 76 &&[26] _Bool [sh&&->25] == *(@-20) 70 &&[25]  "
	mov	al, [ebp-8]
	movzx	eax, al
	cmp	eax, 127
	sete	al
	movzx	eax, al
; JumpIfZero
	test	eax, eax
	je	L27
	mov	al, [ebp-12]
	movzx	eax, al
	cmp	eax, 69
	sete	al
	movzx	eax, al
L27:
	test	eax, eax
	setne	al
	movsx	eax, al
; JumpIfZero
	test	eax, eax
	je	L26
	mov	al, [ebp-16]
	movzx	eax, al
	cmp	eax, 76
	sete	al
	movzx	eax, al
L26:
	test	eax, eax
	setne	al
	movsx	eax, al
; JumpIfZero
	test	eax, eax
	je	L25
	mov	al, [ebp-20]
	movzx	eax, al
	cmp	eax, 70
	sete	al
	movzx	eax, al
L25:
; JumpIfZero
	test	eax, eax
	je	L23
; {
; RPN'ized expression: "( 88 putch ) "
; Expanded expression: " 88  putch ()4 "
; Fused expression:    "( 88 , putch )4 "
	push	88
	call	_putch
	sub	esp, -4
; break
	jmp	L18
; }
L23:
; RPN'ized expression: "elfaddr ++p "
; Expanded expression: "(@-4) ++p(4) "
; Fused expression:    "++p(4) *(@-4) "
	mov	eax, [ebp-4]
	inc	dword [ebp-4]
; }
	jmp	L17
L18:

section .rodata
L28:
	db	"Found ELF header",10
	times	1 db 0

section .text
; RPN'ized expression: "( L28 printstring ) "
; Expanded expression: " L28  printstring ()4 "
; Fused expression:    "( L28 , printstring )4 "
	push	L28
	call	_printstring
	sub	esp, -4
; loc     header : (@-8) : * struct <something>
; loc     <something> : * struct <something>
; RPN'ized expression: "header elfaddr (something29) = "
; Expanded expression: "(@-8) (@-4) *(4) =(4) "
; Fused expression:    "=(204) *(@-8) *(@-4) "
	mov	eax, [ebp-4]
	mov	[ebp-8], eax
; loc     sections : (@-12) : * struct <something>
; loc     <something> : * struct <something>
; loc     <something> : int
; RPN'ized expression: "sections elfaddr (something31) header e_shoff -> *u + (something30) = "
; Expanded expression: "(@-12) (@-4) *(4) (@-8) *(4) 32 + *(4) + =(4) "
; Fused expression:    "+ *(@-8) 32 + *(@-4) *ax =(204) *(@-12) ax "
	mov	eax, [ebp-8]
	add	eax, 32
	mov	ebx, eax
	mov	ecx, [ebx]
	mov	eax, [ebp-4]
	add	eax, ecx
	mov	[ebp-12], eax
; for
; loc     i : (@-16) : unsigned
; RPN'ized expression: "i 0 = "
; Expanded expression: "(@-16) 0 =(4) "
; Fused expression:    "=(204) *(@-16) 0 "
	mov	eax, 0
	mov	[ebp-16], eax
L32:
; RPN'ized expression: "i header e_shnum -> *u < "
; Expanded expression: "(@-16) *(4) (@-8) *(4) 48 + *(2) <u "
; Fused expression:    "+ *(@-8) 48 <u *(@-16) *ax IF! "
	mov	eax, [ebp-8]
	add	eax, 48
	mov	ebx, eax
	movzx	ecx, word [ebx]
	mov	eax, [ebp-16]
	cmp	eax, ecx
	jae	L35
; RPN'ized expression: "i ++p "
; Expanded expression: "(@-16) ++p(4) "
; {
; loc         section : (@-56) : struct <something>
; RPN'ized expression: "section sections i + *u = "
; Expanded expression: " (@-56)  (@-12) *(4) (@-16) *(4) 40 * +  40u  L36 ()12 "
; Fused expression:    "( (@-56) , * *(@-16) 40 + *(@-12) ax , 40u , L36 )12 "
	lea	eax, [ebp-56]
	push	eax
	mov	eax, [ebp-16]
	imul	eax, eax, 40
	mov	ecx, eax
	mov	eax, [ebp-12]
	add	eax, ecx
	push	eax
	push	40
	call	L36
	sub	esp, -12
; if
; RPN'ized expression: "section &u sh_addr -> *u "
; Expanded expression: "(@-44) *(4) "
; Fused expression:    "*(4) (@-44)  "
	mov	eax, [ebp-44]
; JumpIfZero
	test	eax, eax
	je	L37
; {
; if
; RPN'ized expression: "section &u sh_type -> *u 1 == "
; Expanded expression: "(@-52) *(4) 1 == "
; Fused expression:    "== *(@-52) 1 IF! "
	mov	eax, [ebp-52]
	cmp	eax, 1
	jne	L39
; {
; for
; loc                 e : (@-60) : unsigned
; RPN'ized expression: "e 0 = "
; Expanded expression: "(@-60) 0 =(4) "
; Fused expression:    "=(204) *(@-60) 0 "
	mov	eax, 0
	mov	[ebp-60], eax
L41:
; RPN'ized expression: "e section &u sh_size -> *u < "
; Expanded expression: "(@-60) *(4) (@-36) *(4) <u "
; Fused expression:    "<u *(@-60) *(@-36) IF! "
	mov	eax, [ebp-60]
	cmp	eax, [ebp-36]
	jae	L44
; RPN'ized expression: "e ++p "
; Expanded expression: "(@-60) ++p(4) "
; {
; loc                     <something> : * char
; loc                     <something> : * char
; RPN'ized expression: "section &u sh_addr -> *u (something45) e + *u elfaddr (something46) section &u sh_offset -> *u + e + *u = "
; Expanded expression: "(@-44) *(4) (@-60) *(4) + (@-4) *(4) (@-40) *(4) + (@-60) *(4) + *(-1) =(-1) "
; Fused expression:    "+ *(@-44) *(@-60) push-ax + *(@-4) *(@-40) + ax *(@-60) =(119) **sp *ax "
	mov	eax, [ebp-44]
	add	eax, [ebp-60]
	push	eax
	mov	eax, [ebp-4]
	add	eax, [ebp-40]
	add	eax, [ebp-60]
	mov	ebx, eax
	mov	al, [ebx]
	movsx	eax, al
	pop	ebx
	mov	[ebx], al
	movsx	eax, al
; }
L42:
; Fused expression:    "++p(4) *(@-60) "
	mov	eax, [ebp-60]
	inc	dword [ebp-60]
	jmp	L41
L44:
; }
L39:
; }
L37:
; }
L33:
; Fused expression:    "++p(4) *(@-16) "
	mov	eax, [ebp-16]
	inc	dword [ebp-16]
	jmp	L32
L35:
; loc     foo : (@-16) : * () * void
; loc     <something> : * void
; RPN'ized expression: "foo header e_entry -> *u (something47) = "
; Expanded expression: "(@-16) (@-8) *(4) 24 + *(4) =(4) "
; Fused expression:    "+ *(@-8) 24 =(204) *(@-16) *ax "
	mov	eax, [ebp-8]
	add	eax, 24
	mov	ebx, eax
	mov	eax, [ebx]
	mov	[ebp-16], eax
; RPN'ized expression: "( foo ) "
; Expanded expression: " (@-16) *(4) ()0 "
; Fused expression:    "( *(4) (@-16) )0 "
	mov	eax, [ebp-16]
	call	eax
; for
L48:
	jmp	L48
L51:
L15:
	leave
	ret

section .text
L36:
	push	ebp
	mov	ebp, esp
	;sub	esp,          0
	mov	edi, [ebp+16]
	mov	esi, [ebp+12]
	mov	ecx, [ebp+8]
	cld
	rep	movsb
	mov	eax, [ebp+16]
	leave
	ret



; Syntax/declaration table/stack:
; Bytes used: 665/15360


; Macro table:
; Macro __SMALLER_C__ = `0x0100`
; Macro __SMALLER_C_32__ = ``
; Macro __SMALLER_C_SCHAR__ = ``
; Macro __SMALLER_C_UWCHAR__ = ``
; Macro __SMALLER_C_WCHAR16__ = ``
; Macro ELFMAG0 = `0x7f`
; Macro ELFMAG1 = `'E'`
; Macro ELFMAG2 = `'L'`
; Macro ELFMAG3 = `'F'`
; Macro EI_NIDENT = `16`
; Bytes used: 177/5120


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
; Ident Elf32_Word
; Ident Elf32_Addr
; Ident Elf32_Off
; Ident Elf32_Sword
; Ident Elf32_Half
; Ident <something>
; Ident e_ident
; Ident e_type
; Ident e_machine
; Ident e_version
; Ident e_entry
; Ident e_phoff
; Ident e_shoff
; Ident e_flags
; Ident e_ehsize
; Ident e_phentsize
; Ident e_phnum
; Ident e_shentsize
; Ident e_shnum
; Ident e_shstrndx
; Ident ELFHEADER
; Ident sh_name
; Ident sh_type
; Ident sh_flags
; Ident sh_addr
; Ident sh_offset
; Ident sh_size
; Ident sh_link
; Ident sh_info
; Ident sh_addralign
; Ident sh_entsize
; Ident ELFSECTION
; Ident x
; Ident y
; Ident putch
; Ident d
; Ident printstring
; Ident message
; Ident kernelpreloader
; Bytes used: 516/5632

; Next label number: 52
; Compilation succeeded.
