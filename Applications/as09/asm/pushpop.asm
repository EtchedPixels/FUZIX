	PUSH	AL		; illeg
	PUSH	AH		; illeg
	PUSH	BL		; illeg
	PUSH	BH		; illeg
	PUSH	CL		; illeg
	PUSH	CH		; illeg
	PUSH	DL		; illeg
	PUSH	DH		; illeg
	PUSH	#1		; illeg
	PUSH	BYTE #1		; illeg
	PUSH	[BX]		; illeg
	PUSH	BYTE [BX]	; illeg
	PUSH	WORD #-1	; right way to push a signed byte value

	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SP
	PUSH	BP
	PUSH	SI
	PUSH	DI
	PUSH	CS
	PUSH	DS
	PUSH	ES
	PUSH	FS
	PUSH	GS
	PUSH	SS
	PUSH	#$1234		; illeg
	PUSH	WORD #$1234
	PUSH	WORD [BX]

	PUSH	EAX
	PUSH	EBX
	PUSH	ECX
	PUSH	EDX
	PUSH	ESP
	PUSH	EBP
	PUSH	ESI
	PUSH	EDI
	PUSH	#$12345678	; illeg
	PUSH	DWORD #$12345678
	PUSH	DWORD [BX]

	POP	AL		; illeg
	POP	AH		; illeg
	POP	BL		; illeg
	POP	BH		; illeg
	POP	CL		; illeg
	POP	CH		; illeg
	POP	DL		; illeg
	POP	DH		; illeg
	POP	#1		; illeg
	POP	BYTE #1		; illeg
	POP	[BX]		; illeg
	POP	BYTE [BX]	; illeg

	POP	AX
	POP	BX
	POP	CX
	POP	DX
	POP	SP
	POP	BP
	POP	SI
	POP	DI
	POP	CS		; illeg
	POP	DS
	POP	ES
	POP	FS
	POP	GS
	POP	SS
	POP	#$1234		; illeg
	POP	WORD #$1234	; illeg
	POP	WORD [BX]

	POP	EAX
	POP	EBX
	POP	ECX
	POP	EDX
	POP	ESP
	POP	EBP
	POP	ESI
	POP	EDI
	POP	#$12345678	; illeg
	POP	DWORD #$12345678	; illeg
	POP	DWORD [BX]
