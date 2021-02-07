bits 32

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
; loc     ptr : (@-4) : int
; RPN'ized expression: "ptr y 25 * x + 2 * = "
; Expanded expression: "(@-4) y *(1) 25 * x *(1) + 2 * =(4) "
; Fused expression:    "* *y 25 + ax *x * ax 2 =(204) *(@-4) ax "
	mov	al, [_y]
	movzx	eax, al
	imul	eax, eax, 25
	movzx	ecx, byte [_x]
	add	eax, ecx
	imul	eax, eax, 2
	mov	[ebp-4], eax
; loc     <something> : * unsigned char
; RPN'ized expression: "753664 (something3) ptr + *u d = "
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
; RPN'ized expression: "753664 (something4) ptr 1 + + *u 96 = "
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
L1:
	leave
	ret

; glb kernelpreloader : () void
section .text
	global	_kernelpreloader
_kernelpreloader:
	push	ebp
	mov	ebp, esp
	 sub	esp,         16
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
L7:
; {
; loc         A : (@-8) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "A elfaddr 0 + (something9) 0 + *u = "
; Expanded expression: "(@-8) (@-4) *(4) 0 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 0 =(201) *(@-8) *ax "
	mov	eax, [ebp-4]
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-8], eax
; loc         B : (@-12) : unsigned char
; loc         <something> : * unsigned char
; RPN'ized expression: "B elfaddr 1 + (something10) 0 + *u = "
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
; RPN'ized expression: "C elfaddr 2 + (something11) 0 + *u = "
; Expanded expression: "(@-16) (@-4) *(4) 2 + *(1) =(1) "
; Fused expression:    "+ *(@-4) 2 =(201) *(@-16) *ax "
	mov	eax, [ebp-4]
	add	eax, 2
	mov	ebx, eax
	mov	al, [ebx]
	movzx	eax, al
	mov	[ebp-16], eax
; if
; RPN'ized expression: "A 69 == B 76 == && C 70 == && "
; Expanded expression: "(@-8) *(1) 69 == [sh&&->15] (@-12) *(1) 76 == &&[15] _Bool [sh&&->14] (@-16) *(1) 70 == &&[14] "
; Fused expression:    "== *(@-8) 69 [sh&&->15] == *(@-12) 76 &&[15] _Bool [sh&&->14] == *(@-16) 70 &&[14]  "
	mov	al, [ebp-8]
	movzx	eax, al
	cmp	eax, 69
	sete	al
	movzx	eax, al
; JumpIfZero
	test	eax, eax
	je	L15
	mov	al, [ebp-12]
	movzx	eax, al
	cmp	eax, 76
	sete	al
	movzx	eax, al
L15:
	test	eax, eax
	setne	al
	movsx	eax, al
; JumpIfZero
	test	eax, eax
	je	L14
	mov	al, [ebp-16]
	movzx	eax, al
	cmp	eax, 70
	sete	al
	movzx	eax, al
L14:
; JumpIfZero
	test	eax, eax
	je	L12
; {
; RPN'ized expression: "( 88 putch ) "
; Expanded expression: " 88  putch ()4 "
; Fused expression:    "( 88 , putch )4 "
	push	88
	call	_putch
	sub	esp, -4
; break
	jmp	L8
; }
L12:
; RPN'ized expression: "elfaddr ++p "
; Expanded expression: "(@-4) ++p(4) "
; Fused expression:    "++p(4) *(@-4) "
	mov	eax, [ebp-4]
	inc	dword [ebp-4]
; }
	jmp	L7
L8:
; for
L16:
	jmp	L16
L19:
L5:
	leave
	ret



; Syntax/declaration table/stack:
; Bytes used: 125/15360


; Macro table:
; Macro __SMALLER_C__ = `0x0100`
; Macro __SMALLER_C_32__ = ``
; Macro __SMALLER_C_SCHAR__ = ``
; Macro __SMALLER_C_UWCHAR__ = ``
; Macro __SMALLER_C_WCHAR16__ = ``
; Bytes used: 110/5120


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
; Ident x
; Ident y
; Ident putch
; Ident d
; Ident kernelpreloader
; Bytes used: 156/5632

; Next label number: 20
; Compilation succeeded.
