; group6.asm
; 0F 00 /nnn

;	LLDT	r/m16		nnn = 010
;	LTR	r/m16		nnn = 011
;	SLDT	r/m16		nnn = 000
;	STR	r/m16		nnn = 001
;	VERR	r/m16		nnn = 100
;	VERW	r/m16		nnn = 101

	LLDT	AL		; illeg size
	LLDT	EAX		; illeg size
	LLDT	WORD $1234	; immed not allowed
	LLDT	DS		; segreg not allowed

	LLDT	AX
	LLDT	[BX]
	LLDT	[EAX]

	LTR	BX
	SLDT	[BP]
	STR	[EBX]
	VERR	CX
	VERW	[SI]
