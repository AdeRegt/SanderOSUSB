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
; glb relocate : (
; prm     elfaddr : unsigned
; prm     section : struct <something>
;     ) void
section .text
	global	_relocate
_relocate:
	push	ebp
	mov	ebp, esp
	 sub	esp,         16
; loc     elfaddr : (@8) : unsigned
; loc     section : (@12) : struct <something>
; loc     e : (@-4) : unsigned
; RPN'ized expression: "e 0 = "
; Expanded expression: "(@-4) 0 =(4) "
; Fused expression:    "=(204) *(@-4) 0 "
	mov	eax, 0
	mov	[ebp-4], eax
; loc     oldaddr : (@-8) : unsigned
; RPN'ized expression: "oldaddr 0 = "
; Expanded expression: "(@-8) 0 =(4) "
; Fused expression:    "=(204) *(@-8) 0 "
	mov	eax, 0
	mov	[ebp-8], eax
; loc     newaddr : (@-12) : unsigned
; RPN'ized expression: "newaddr 0 = "
; Expanded expression: "(@-12) 0 =(4) "
; Fused expression:    "=(204) *(@-12) 0 "
	mov	eax, 0
	mov	[ebp-12], eax
; loc     byteold : (@-16) : unsigned char
; RPN'ized expression: "byteold 0 = "
; Expanded expression: "(@-16) 0 =(1) "
; Fused expression:    "=(204) *(@-16) 0 "
	mov	eax, 0
	mov	[ebp-16], eax
; onceagain:
L3:
; RPN'ized expression: "oldaddr elfaddr section &u sh_offset -> *u + e + = "
; Expanded expression: "(@-8) (@8) *(4) (@28) *(4) + (@-4) *(4) + =(4) "
; Fused expression:    "+ *(@8) *(@28) + ax *(@-4) =(204) *(@-8) ax "
	mov	eax, [ebp+8]
	add	eax, [ebp+28]
	add	eax, [ebp-4]
	mov	[ebp-8], eax
; RPN'ized expression: "newaddr section &u sh_addr -> *u e + = "
; Expanded expression: "(@-12) (@24) *(4) (@-4) *(4) + =(4) "
; Fused expression:    "+ *(@24) *(@-4) =(204) *(@-12) ax "
	mov	eax, [ebp+24]
	add	eax, [ebp-4]
	mov	[ebp-12], eax
; loc     <something> : * unsigned char
; RPN'ized expression: "byteold oldaddr (something4) 0 + *u = "
; Expanded expression: "(@-16) (@-8) *(4) 0 + *(1) =(1) "
; Fused expression:    "+ *(@-8) 0 =(153) *(@-16) *ax "
	mov	eax, [ebp-8]
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-16], al
	movzx	eax, al
; loc     <something> : * unsigned char
; RPN'ized expression: "newaddr (something5) 0 + *u byteold = "
; Expanded expression: "(@-12) *(4) 0 + (@-16) *(1) =(1) "
; Fused expression:    "+ *(@-12) 0 =(153) *ax *(@-16) "
	mov	eax, [ebp-12]
	mov	ebx, eax
	mov	al, [ebp-16]
	movzx	eax, al
	mov	[ebx], al
	movzx	eax, al
; if
; RPN'ized expression: "e section &u sh_size -> *u < "
; Expanded expression: "(@-4) *(4) (@32) *(4) <u "
; Fused expression:    "<u *(@-4) *(@32) IF! "
	mov	eax, [ebp-4]
	cmp	eax, [ebp+32]
	jae	L6
; {
; RPN'ized expression: "e ++p "
; Expanded expression: "(@-4) ++p(4) "
; Fused expression:    "++p(4) *(@-4) "
	mov	eax, [ebp-4]
	inc	dword [ebp-4]
; goto onceagain
	jmp	L3
; }
L6:
L1:
	leave
	ret

; glb header : * struct <something>
section .bss
	alignb 4
	global	_header
_header:
	resb	4

; glb kernelpreloader : () void
section .text
	global	_kernelpreloader
_kernelpreloader:
	push	ebp
	mov	ebp, esp
	 sub	esp,         56
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
L10:
; {
; loc         A : (@-8) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "A elfaddr 0 + (something12) 0 + *u = "
; Expanded expression: "(@-8) (@-4) *(4) 0 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 0 =(201) *(@-8) *ax "
	mov	eax, [ebp-4]
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-8], eax
; loc         B : (@-12) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "B elfaddr 1 + (something13) 0 + *u = "
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
; RPN'ized expression: "C elfaddr 2 + (something14) 0 + *u = "
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
; RPN'ized expression: "D elfaddr 3 + (something15) 0 + *u = "
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
; Expanded expression: "(@-8) *(1) 127 == [sh&&->20] (@-12) *(1) 69 == &&[20] _Bool [sh&&->19] (@-16) *(1) 76 == &&[19] _Bool [sh&&->18] (@-20) *(1) 70 == &&[18] "
; Fused expression:    "== *(@-8) 127 [sh&&->20] == *(@-12) 69 &&[20] _Bool [sh&&->19] == *(@-16) 76 &&[19] _Bool [sh&&->18] == *(@-20) 70 &&[18]  "
	mov	al, [ebp-8]
	movzx	eax, al
	cmp	eax, 127
	sete	al
	movzx	eax, al
; JumpIfZero
	test	eax, eax
	je	L20
	mov	al, [ebp-12]
	movzx	eax, al
	cmp	eax, 69
	sete	al
	movzx	eax, al
L20:
	test	eax, eax
	setne	al
	movsx	eax, al
; JumpIfZero
	test	eax, eax
	je	L19
	mov	al, [ebp-16]
	movzx	eax, al
	cmp	eax, 76
	sete	al
	movzx	eax, al
L19:
	test	eax, eax
	setne	al
	movsx	eax, al
; JumpIfZero
	test	eax, eax
	je	L18
	mov	al, [ebp-20]
	movzx	eax, al
	cmp	eax, 70
	sete	al
	movzx	eax, al
L18:
; JumpIfZero
	test	eax, eax
	je	L16
; {
; goto t1
	jmp	L21
; }
L16:
; RPN'ized expression: "elfaddr ++p "
; Expanded expression: "(@-4) ++p(4) "
; Fused expression:    "++p(4) *(@-4) "
	mov	eax, [ebp-4]
	inc	dword [ebp-4]
; }
	jmp	L10
L11:
; t1:
L21:
; loc     <something> : * struct <something>
; RPN'ized expression: "header elfaddr (something22) = "
; Expanded expression: "header (@-4) *(4) =(4) "
; Fused expression:    "=(204) *header *(@-4) "
	mov	eax, [ebp-4]
	mov	[_header], eax
; loc     sections : (@-8) : * struct <something>
; loc     <something> : * struct <something>
; loc     <something> : int
; RPN'ized expression: "sections elfaddr (something24) header e_shoff -> *u + (something23) = "
; Expanded expression: "(@-8) (@-4) *(4) header *(4) 32 + *(4) + =(4) "
; Fused expression:    "+ *header 32 + *(@-4) *ax =(204) *(@-8) ax "
	mov	eax, [_header]
	add	eax, 32
	mov	ebx, eax
	mov	ecx, [ebx]
	mov	eax, [ebp-4]
	add	eax, ecx
	mov	[ebp-8], eax
; loc     headerc : (@-12) : unsigned short
; RPN'ized expression: "headerc header e_shnum -> *u = "
; Expanded expression: "(@-12) header *(4) 48 + *(2) =(2) "
; Fused expression:    "+ *header 48 =(202) *(@-12) *ax "
	mov	eax, [_header]
	add	eax, 48
	mov	ebx, eax
	mov	ax, [ebx]
	movzx	eax, ax
	mov	[ebp-12], eax
; for
; loc     i : (@-16) : unsigned short
; RPN'ized expression: "i 0 = "
; Expanded expression: "(@-16) 0 =(2) "
; Fused expression:    "=(204) *(@-16) 0 "
	mov	eax, 0
	mov	[ebp-16], eax
L25:
; RPN'ized expression: "i headerc < "
; Expanded expression: "(@-16) *(2) (@-12) *(2) < "
; Fused expression:    "< *(@-16) *(@-12) IF! "
	mov	ax, [ebp-16]
	movzx	eax, ax
	movzx	ecx, word [ebp-12]
	cmp	eax, ecx
	jge	L28
; RPN'ized expression: "i ++p "
; Expanded expression: "(@-16) ++p(2) "
; {
; loc         section : (@-56) : struct <something>
; RPN'ized expression: "section sections i + *u = "
; Expanded expression: " (@-56)  (@-8) *(4) (@-16) *(2) 40 * +  40u  L29 ()12 "
; Fused expression:    "( (@-56) , * *(@-16) 40 + *(@-8) ax , 40u , L29 )12 "
	lea	eax, [ebp-56]
	push	eax
	mov	ax, [ebp-16]
	movzx	eax, ax
	imul	eax, eax, 40
	mov	ecx, eax
	mov	eax, [ebp-8]
	add	eax, ecx
	push	eax
	push	40
	call	L29
	sub	esp, -12
; if
; RPN'ized expression: "section &u sh_addr -> *u "
; Expanded expression: "(@-44) *(4) "
; Fused expression:    "*(4) (@-44)  "
	mov	eax, [ebp-44]
; JumpIfZero
	test	eax, eax
	je	L30
; {
; if
; RPN'ized expression: "section &u sh_type -> *u 1 == "
; Expanded expression: "(@-52) *(4) 1 == "
; Fused expression:    "== *(@-52) 1 IF! "
	mov	eax, [ebp-52]
	cmp	eax, 1
	jne	L32
; {
; RPN'ized expression: "( section , elfaddr relocate ) "
; Expanded expression: "  40u  (@-56)  L34 ()8  (@-4) *(4)  relocate ()44 "
; Fused expression:    "( ( 40u , (@-56) , L34 )8 , *(4) (@-4) , relocate )44 "
	push	40
	lea	eax, [ebp-56]
	push	eax
	call	L34
	sub	esp, -8
	push	eax
	push	dword [ebp-4]
	call	_relocate
	sub	esp, -44
; }
L32:
; }
L30:
; }
L26:
; Fused expression:    "++p(2) *(@-16) "
	mov	ax, [ebp-16]
	movzx	eax, ax
	inc	word [ebp-16]
	jmp	L25
L28:
; loc     foo : (@-16) : * () * void
; loc     <something> : * void
; RPN'ized expression: "foo header e_entry -> *u (something35) = "
; Expanded expression: "(@-16) header *(4) 24 + *(4) =(4) "
; Fused expression:    "+ *header 24 =(204) *(@-16) *ax "
	mov	eax, [_header]
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
L36:
	jmp	L36
L39:
L8:
	leave
	ret

section .text
L29:
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

section .text
L34:
	push	ebp
	mov	ebp, esp
	;sub	esp,          0
	mov	edx, [ebp+4]
	mov	esi, [ebp+8]
	mov	ecx, [ebp+12]
	mov	ebp, [ebp]
	lea	eax, [ecx + 3]
	and	eax, -4
	sub	eax, 4*4
	sub	esp, eax
	lea	esi, [esi + ecx - 1]
	lea	edi, [esp + ecx - 1]
	std
	rep	movsb
	cld
	mov	eax, [esp]
	push	0
	push	edx
	ret



; Syntax/declaration table/stack:
; Bytes used: 635/15360


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
; Ident relocate
; Ident elfaddr
; Ident section
; Ident header
; Ident kernelpreloader
; Bytes used: 514/5632

; Next label number: 40
; Compilation succeeded.
