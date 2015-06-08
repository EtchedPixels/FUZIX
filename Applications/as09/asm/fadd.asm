_fadd:
	PUSH	BP
	MOV	BP,SP
	MOV	EAX,DWORD PTR [BP+4]
	MOV	EDX,DWORD PTR [BP+8]
	MOV	EBX,DWORD PTR [BP+12]
	MOV	ECX,DWORD PTR [BP+16]
	CALL	faddfxfy
	MOV	DWORD PTR _facc,EAX
	MOV	DWORD PTR _facc+4,EDX
	POP	BP
	RET

fsubfxfy:
	XOR	ECX,#$80000000	; complement sign bit, fall into add routine
faddfxfy:
	PUSH	EBP
	PUSH	EDI
	PUSH	ESI
	MOV	EDI,ECX		; free CL for shifts
	MOV	ESI,EDX		; this mainly for consistent naming
	AND	ESI,#$7FFFFFFF	; discard sign so comparison is simple
	AND	EDI,#$7FFFFFFF

	CMP	ESI,EDI
	JA	XBIG
	JB	SWAP
	CMP	EAX,EBX
	JAE	XBIG
SWAP:
	XCHG	EDX,ECX
	XCHG	ESI,EDI
	XCHG	EAX,EBX
XBIG:
	AND	ESI,#$000FFFFF	; discard exponent
	AND	EDI,#$000FFFFF
	OR	ESI,#$00100000	; normalize
	OR	EDI,#$00100000

	SHR	ECX,32-(1+11)
	SHR	EDX,32-(1+11)	
	MOV	EBP,ECX		; prepare to compare signs (want high bits 0)
	SUB	CX,DX		; get difference of signs in CX
	NEG	CX		; D holds sign and exponent of both throughout
	CMP	CX,#(64-11)+2
	JAE	TO_DONE1	; x dominates y
	XOR	BP,DX
	AND	BP,#$0800	; see if signs are same
	JNZ	TO_SUBTRACT	; else roundoff reg EBP is 0

	CMP	CL,#32
	JAE	TO_ADD_BIGSHIFT
	SHRD	EBP,EBX,CL
	SHRD	EBX,EDI,CL
	SHR	EDI,CL
	ADD	EAX,EBX
	ADC	ESI,EDI
	SUB	EBX,EBX

; result DX(1+11):SI:AX:BP:BX but needs normalization

NORMALIZE:
	MOV	CX,DX
	AND	CX,#$07FF
	TEST	ESI,#$00200000
	JZ	NORMALIZE2
	BR	LOVERFLOW

TO_DONE1:
	JMP	DONE1

TO_SUBTRACT:
	BR	SUBTRACT

TO_ADD_BIGSHIFT:
	BR	ADD_BIGSHIFT

TO_NORMLITTLE:
	BR	NORMLITTLE

; result DX(1):CX(11):SI:AX:BP:BX

NORMALIZE2:
	SHRD	EDI,ESI,32-11
				; top 11 bits of ESI known 0 and BSR is slooow
	BSR	EDI,EDI		; index of leading 1 bit in EDI is 11..31 in DI
	JZ	TO_NORMLITTLE	; ESI is zero (flag wrong in Intel Manual)
	SUB	DI,#31
	NEG	DI
	PUSH	CX		; gr
	MOV	CX,DI		; rr
	SHLD	ESI,EAX,CL
	SHLD	EAX,EBP,CL
	SHLD	EBP,EBX,CL
	SHL	EBX,CL
	POP	CX		; rr
	SUB	CX,DI
	JC	UNDERFLOW	

ROUND:
	CMP	EBP,#$80000000	; test roundoff register
	JA	ROUNDUP
	JB	DONE		; no rounding
	TEST	EBX,EBX
	JNZ	ROUNDUP
	TEST	AL,#1		; ambiguous case, round to even
	JZ	DONE		; even, no rounding
ROUNDUP:
	ADD	EAX,#1
	ADC	ESI,#0
	SUB	EBP,EBP
	SUB	EBX,EBX
	TEST	ESI,#$00200000
	JNZ	LOVERFLOW	; rounding may cause overflow!

DONE:
	AND	DX,#$0800	; extract sign of largest and result
	OR	DX,CX		; include exponent with sign
DONE1:
	SHL	EDX,32-(1+11)
	AND	ESI,#$000FFFFF	; discard normalization bit
	OR	EDX,ESI	
	POP	ESI
	POP	EDI
	POP	EBP
	RET

UNDERFLOW:				; should have error message here
ANSWER0:
	SUB	EDX,EDX
	MOV	EAX,EDX
	POP	ESI
	POP	EDI
	POP	EBP
	RET

LOVERFLOW:			; carry bit must be right-shifted back in
	SHR	ESI,1
	RCR	EAX,1
	RCR	EBP,1
	RCR	EBX,1
	INC	CX
	CMP	CX,#$0800
	JNZ	ROUND

OVERFLOW:			; should have error message here
	MOV	EDX,#$FFE00000	; + infinity
	SUB	EAX,EAX
	POP	ESI
	POP	EDI
	POP	EBP
	RET

ADD_BIGSHIFT:
	SUB	CL,#32
	SHRD	EBP,EBX,CL
	SHRD	EBX,EDI,CL
	SHR	EDI,CL
	ADD	EAX,EDI
	ADC	ESI,#0
	XCHG	EBP,EBX
	BR	NORMALIZE

NORMLITTLE:
	SHLD	ESI,EAX,32-(1+11)
	SHLD	EAX,EBP,32-(1+11)
	SHLD	EBP,EBX,32-(1+11)
	SHL	EBX,20
	SUB	CL,#32-(1+11)
	JC	UNDERFLOW
	BR	NORMALIZE2

SUBTRACT:
	SUB	EBP,EBP		; set up roundoff register
	CMP	CL,#32
	JAE	SUBTRACT_BIGSHIFT
	SHRD	EBP,EBX,CL
	SHRD	EBX,EDI,CL
	SHR	EDI,CL
	NEG	EBP
	SBB	EAX,EBX
	SBB	ESI,EDI
	SUB	EBX,EBX
	MOV	CX,DX
	AND	CX,#$07FF
	BR	NORMALIZE2

SUBTRACT_BIGSHIFT:
	SUB	CL,#32
	SHRD	EBP,EBX,CL
	SHRD	EBX,EDI,CL
	SHR	EDI,CL
	NEG	EBX
	NEG	EBP
	SBB	EBX,#0
	SBB	EAX,EDI
	SBB	ESI,#0
	XCHG	EBP,EBX
	MOV	CX,DX
	AND	CX,#$07FF
	BR	NORMALIZE2

TO_ANSWER0:
	BR	ANSWER0

TO_OVERFLOW:
	JMP	TO_OVERFLOW

TO_UNDERFLOW:
	BR	UNDERFLOW

fmulfxfy:
	PUSH	EBP
	PUSH	EDI
	PUSH	ESI
	MOV	ESI,EDX		; free DX for multiplications
	MOV	EDI,ECX		; this mainly for consistent naming
	SHR	EDX,32-(1+11)
	SHR	ECX,32-(1+11)	
	MOV	BP,DX
	XOR	BP,CX
	AND	BP,#$0800	; extract sign
	AND	DX,#$07FF	; exp(x)
	JZ	TO_ANSWER0
	AND	CX,#$07FF	; exp(y)
	JZ	TO_ANSWER0
	ADD	CX,DX
	SUB	CX,#$0400
	JB	TO_UNDERFLOW
	CMP	CX,#$07FF
	JA	TO_OVERFLOW	; probably not quite right

	AND	ESI,#$000FFFFF	; discard sign and exponent
	AND	EDI,#$000FFFFF
	OR	ESI,#$00100000	; normalize
	OR	EDI,#$00100000

; exponent is in CX, sign in BP, operands in ESI:EAX and EDI:EBX, DX is free
; product to go in ESI:EAX:EBP:EBX
; terminology: x * y = (x32:x0) * (y32:y0) = x32y32 + x32y0 + x0y32 +x0y0

	PUSH	CX
	PUSH	BP
	MOV	ECX,EAX
	MUL	EBX		; x0y0
	MOV	EBP,EDX		; x0y0.high in EBP
	XCHG	EBX,EAX		; x0y0.low in EBX (final), y0 in EAX
	MUL	ESI		; x32y0
	PUSH	EAX		; x32y0.low on stack
	PUSH	EDX		; x32y0.high on stack
	MOV	EAX,ESI
	MUL	EDI		; x32y32
	MOV	ESI,EDX		; x32y32.high in ESI (final except carries)
	XCHG	ECX,EAX		; x32y32.low in ECX, x0 in EAX
	MUL	EDI		; x0y32

	ADD	EBP,EAX		; x0y0.high + x0y32.low
	POP	EAX		; x32y0.high
	ADC	EAX,EDX		; x32y0.high + x0y32.high
	ADC	ESI,#0
	POP	EDX		; x32y0.low
	ADD	EBP,EDX		; (x0y0.high + x0y32.low) + x32y0.low
	ADC	EAX,ECX		; (x32y0.high + x0y32.high) + x32y32.low
	ADC	ESI,#0
	POP	DX		; sign
	POP	CX		; exponent
	ADD	CX,#13		; temp fixup
	BR	NORMALIZE2

_facc:
	.word	0,0
