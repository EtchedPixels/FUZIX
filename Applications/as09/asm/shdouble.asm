; SHDOUBLE.ASM

ILLEGALS	EQU	1

; 0F A4		SHLD	r/m16,r16,imm8		3/7
; 0F A4		SHLD	r/m32,r32,imm8		3/7
; 0F A5		SHLD	r/m16,r16,CL		3/7
; 0F A5		SHLD	r/m32,r32,CL		3/7

; 0F AC		SHRD	r/m16,r16,imm8		3/7
; 0F AC		SHRD	r/m32,r32,imm8		3/7
; 0F AD		SHRD	r/m16,r16,CL		3/7
; 0F AD		SHRD	r/m32,r32,CL		3/7

IF	ILLEGALS
	SHLD	AL,BL,8		; byte size
	SHLD	AX,8,8		; immediate source
	SHLD	AX,DS,8		; segment register
	SHLD	AX,[BX],8	; non-register source
	SHLD	AX,BX,256	; shift count too big
	SHLD	AL,BL,8		; byte size
ENDIF

	SHLD	BX,CX,3
	SHLD	EDX,ESI,1
	SHLD	CX,BX,CL
	SHLD	ESI,EDX,1
	SHLD	[BX],CX,3
	SHLD	[BX],ECX,1
	SHLD	[SI],BX,CL
	SHLD	[SI],EBX,CL

	SHRD	BX,CX,3
	SHRD	CX,BX,CL
