; group7.asm
; 0F 01 /nnn

;	INVLPG	m		nnn = 111
;	LGDT	m16&32		nnn = 010
;	LIDT	m16&32		nnn = 011
;	LMSW	r/m16		nnn = 110
;	SGDT	m		nnn = 000
;	SIDT	m		nnn = 001
;	SMSW	r/m16		nnn = 100

	LGDT	EAX		; register not allowed
	LGDT	#$1234		; immed not allowed
	LGDT	WORD PTR [BX]	; illegal size

	LGDT	[BX]
	LGDT	PWORD PTR [BX]
	LGDT	FWORD PTR [BX]
	LGDT	[EAX]

	INVLPG	[EDI]
	SGDT	[BP]
	SIDT	[EBX]

	LMSW	AL		; illeg size
	LMSW	EAX		; illeg size
	LMSW	#$1234		; immed not allowed
	LMSW	DS		; segreg not allowed

	LMSW	AX
	LMSW	[BX]
	LMSW	[EAX]

	SMSW	BX
