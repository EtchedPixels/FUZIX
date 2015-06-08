general:
		; AL,imm8
		; AX,imm16
		; EAX,imm32
		; r/m8,imm8
		; r/m16,imm16
		; r/m32.imm32
		; r/m16,signed imm8
		; r/m32,signed imm8
		; r/m8,r8
		; r/m16,r16
		; r/m32,r32
		; r8,r/m8
		; r16,r/m16
		; r32,r/m32

shiftcount:
		; 1
		; CL
		; imm8
unary alterable:
		; r/m8
		; r/m16
		; r/m32

	AAA
	AAD	; [unsupported base]
	AAM	; [unsupported base]
	AAS
	ADC	; general
	ADD	; general
	AND	; general
	ARPL	; r/m16,r16
	BOUND	; r16,m16&16
	BOUND	; r32,m32&32
	BSF	; r16,r/m16
	BSF	; r32,r/m32
	BSR	; r16,r/m16
	BSR	; r32,r/m32
	BSWAP	; r32
	BT	; r/m16,r16
	BT	; r/m32,r32
	BT	; r/m16,imm8
	BT	; r/m32,imm8
	BTC	; r/m16,r16
	BTC	; r/m32,r32
	BTC	; r/m16,imm8
	BTC	; r/m32,imm8
	BTR	; r/m16,r16
	BTR	; r/m32,r32
	BTR	; r/m16,imm8
	BTR	; r/m32,imm8
	BTS	; r/m16,r16
	BTS	; r/m32,r32
	BTS	; r/m16,imm8
	BTS	; r/m32,imm8
	CALL	; rel16
	CALL	; r/m16
	CALL	; ptr16:16
	CALL	; m16:16
	CALL	; rel32
	CALL	; r/m32
	CALL	; ptr16:32
	CALL	; m16:32
	CBW
	CDQ
	CLC
	CLD
	CLI
	CLTS
	CMC
	CMP	; general
	CMPS	; [segreg:]m8,m8
	CMPS	; [segreg:]m16,m16
	CMPS	; [segreg:]m32,m32
	CMPSB
	CMPSW
	CMPSD
	CMPXCHG	; r/m8,r8
	CMPXCHG	; r/m16,r16
	CMPXCHG	; r/m32,r32
	CWD
	CWDE
	DAA
	DAS
	DEC	; unary alterable
	DEC	; r16
	DEC	; r32
	DIV	; AL,r/m8
	DIV	; AX,r/m16
	DIV	; EAX,r/m32
	ENTER	; imm16,imm8
	HLT
	IDIV	; AL,r/m8
	IDIV	; AX,r/m16
	IDIV	; EAX,r/m32
	IMUL	; r/m8
	IMUL	; r/m16
	IMUL	; r/m32
	IMUL	; r16,r/m16
	IMUL	; r32,r/m32
	IMUL	; r16,r/m16,imm8
	IMUL	; r32,r/m32,imm8
	IMUL	; r16,imm8
	IMUL	; r32,imm8
	IMUL	; r16,r/m16,imm16
	IMUL	; r32,r/m32,imm32
	IMUL	; r16,imm16
	IMUL	; r32,imm32
	IN	; AL,imm8
	IN	; AX,imm8
	IN	; EAX,imm8
	IN	; AL,DX
	IN	; AX,DX
	IN	; EAX,DX
	INC	; unary alterable
	INC	; r16
	INC	; r32
	INSB
	INSW
	INSD
	INT	; imm8
	INTO
	INVD
	INVLPG	; m
	IRET
	IRETD
	JCC	; rel8
	JCC	; rel16/32
	JA
	JAE
	JB
	JBE
	JC
	JCXZ
	JECXZ
	JE
	JG
	JGE
	JL
	JLE
	JNA
	JNAE
	JNB
	JNBE
	JNC
	JNE
	JNG
	JNGE
	JNL
	JNLE
	JNO
	JNP
	JNS
	JNZ
	JO
	JP
	JPE
	JPO
	JS
	JZ
	JMP	; rel8
	JMP	; rel16
	JMP	; r/m16
	JMP	; ptr16:16
	JMP	; m16:16
	JMP	; rel32
	JMP	; r/m32
	JMP	; ptr16:32
	JMP	; m16:32
	LAHF
	LAR	; r16,r/m16
	LAR	; r32,r/m32
	LEA	; r16,m
	LEA	; r32,m
	LEAVE
	LGDT	; m16&32
	LIDT	; m16&32
	LDS	; r16,m16:16
	LDS	; r32,m16:32
	LES	; r16,m16:16
	LES	; r32,m16:32
	LFS	; r16,m16:16
	LFS	; r32,m16:32
	LGS	; r16,m16:16
	LGS	; r32,m16:32
	LSS	; r16,m16:16
	LSS	; r32,m16:32
	LLDT	; r/m16
	LMSW	; r/m16
	LOCK
	LODS	; [segreg:]m8
	LODS	; [segreg:]m16
	LODS	; [segreg:]m32
	LODSB
	LODSW
	LODSD
	LOOP	; rel8
	LOOPE	; rel8
	LOOPZ	; rel8
	LOOPNE	; rel8
	LOOPNZ	; rel8
	LSL	; r16,r/m16
	LSL	; r32,r/m32
	LTR	; r/m16
	MOV	; r/m8,r8
	MOV	; r/m16,r16
	MOV	; r/m32,r32
	MOV	; r8,r/m8
	MOV	; r16,r/m16
	MOV	; r32,r/m32
	MOV	; r/m16,Sreg
	MOV	; Sreg,r/m16
	MOV	; AL,moffs8
	MOV	; AX,moffs16
	MOV	; EAX,moffs32
	MOV	; moffs8,AL
	MOV	; moffs16,AX
	MOV	; moffs32,EAX
	MOV	; r8,imm8
	MOV	; r16,imm16
	MOV	; r32,imm32
	MOV	; r32,CR0/CR2/CR3
	MOV	; r/m8,imm8
	MOV	; r/m16,imm16
	MOV	; r/m32,imm32
	MOV	; r32,CR0/CR2/CR3
	MOV	; CR0/CR2/CR3,r32
	MOV	; r32,DR0/DR1/DR2/DR3/DR6/DR7
	MOV	; DR0/DR1/DR2/DR3/DR6/DR7,r32
	MOV	; r32,TR6/TR7
	MOV	; TR6/TR7,r32
	MOVS	; [segreg:]m8,m8
	MOVS	; [segreg:]m16,m16
	MOVS	; [segreg:]m32,m32
	MOVSB
	MOVSW
	MOVSD
	MOVSX	; r16,r/m8
	MOVSX	; r32,r/m8
	MOVSX	; r32,r/m16
	MOVZX	; r16,r/m8
	MOVZX	; r32,r/m8
	MOVZX	; r32,r/m16
	MUL	; AL,r/m8
	MUL	; AX,r/m16
	MUL	; EAX,r/m32
	NEG	; unary alterable
	NOP
	NOT	; unary alterable
	OR	; general
	OUT	; imm8,AL
	OUT	; imm8,AX
	OUT	; imm8,EAX
	OUT	; DX,AL
	OUT	; DX,AX
	OUT	; DX,EAX
	OUTS	; [segreg:]m8
	OUTS	; [segreg:]m16
	OUTS	; [segreg:]m32
	OUTSB
	OUTSW
	OUTSD
	POP	; m16
	POP	; m32
	POP	; r16
	POP	; r32
	POP	; DS
	POP	; ES
	POP	; FS
	POP	; GS
	POP	; SS
	POPA
	POPAD
	POPF
	POPFD
	PUSH	; m16
	PUSH	; m32
	PUSH	; r16
	PUSH	; r32
	PUSH	; imm8
	PUSH	; imm16
	PUSH	; imm32
	PUSH	; CS
	PUSH	; DS
	PUSH	; ES
	PUSH	; FS
	PUSH	; GS
	PUSH	; SS
	PUSHA
	PUSHAD
	PUSHF
	PUSHFD
	RCL	; shiftcount
	RCR	; shiftcount
	ROL	; shiftcount
	ROR	; shiftcount
	REP	; INS/MOVS/OUTS/STOS
	REPE	; CMPS/SCAS
	REPNE	; CMPS/SCAS
	RET
	RET	; imm16
	SAHF
	SAL	; shiftcount
	SAR	; shiftcount
	SHL	; shiftcount
	SHR	; shiftcount
	SBB	; general
	SCASB
	SCASW
	SCASD
	SETA	; r/m8
	SETAE	; r/m8
	SETB	; r/m8
	SETBE	; r/m8
	SETC	; r/m8
	SETE	; r/m8
	SETG	; r/m8
	SETGE	; r/m8
	SETL	; r/m8
	SETLE	; r/m8
	SETNA	; r/m8
	SETNAE	; r/m8
	SETNB	; r/m8
	SETNBE	; r/m8
	SETNC	; r/m8
	SETNE	; r/m8
	SETNG	; r/m8
	SETNGE	; r/m8
	SETNL	; r/m8
	SETNLE	; r/m8
	SETNO	; r/m8
	SETNP	; r/m8
	SETNS	; r/m8
	SETNZ	; r/m8
	SETO	; r/m8
	SETP	; r/m8
	SETPE	; r/m8
	SETPO	; r/m8
	SETS	; r/m8
	SETZ	; r/m8
	SGDT	; m
	SHLD	; r/m16,r16,imm8
	SHLD	; r/m32,r32,imm8
	SHLD	; r/m16,r16,CL
	SHLD	; r/m32,r32,CL
	SHRD	; r/m16,r16,imm8
	SHRD	; r/m32,r32,imm8
	SHRD	; r/m16,r16,CL
	SHRD	; r/m32,r32,CL
	SIDT	; m
	SLDT	; r/m16
	SMSW	; r/m16
	STC
	STD
	STI
	STOSB
	STOSW
	STOSD
	STR	; r/m16
	SUB	; general
	TEST	; AL,imm8
	TEST	; AX,imm16
	TEST	; EAX,imm32
	TEST	; r/m8,imm8
	TEST	; r/m16,imm16
	TEST	; r/m32,imm32
	TEST	; r/m8,r8
	TEST	; r/m16,r16
	TEST	; r/m32/r32
	VERR	; r/m16
	VERW	; r/m16
	WAIT
	WBINVD
	XADD	; r/m8,r8
	XADD	; r/m16,r16
	XADD	; r/m32,r32
	XCHG	; AX,r16
	XCHG	; EAX,r32
	XCHG	; r/m8,r8
	XCHG	; r/m16,r16
	XCHG	; r/m32,r32
	XLAT	; [segreg:]m8
	XLATB
	XOR	; general
