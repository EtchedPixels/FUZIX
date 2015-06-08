	INC	AL
	INC	AH
	INC	BL
	INC	BH
	INC	CL
	INC	CH
	INC	DL
	INC	DH
	INC	#1		; illeg
	INC	BYTE #1		; illeg
	INC	[BX]		; illeg
	INC	BYTE [BX]

	INC	AX
	INC	BX
	INC	CX
	INC	DX
	INC	SP
	INC	BP
	INC	SI
	INC	DI
	INC	CS		; illeg
	INC	DS		; illeg
	INC	ES		; illeg
	INC	FS		; illeg
	INC	GS		; illeg
	INC	#$1234		; illeg
	INC	WORD #$1234	; illeg
	INC	WORD [BX]

	INC	EAX
	INC	EBX
	INC	ECX
	INC	EDX
	INC	ESP
	INC	EBP
	INC	ESI
	INC	EDI
	INC	#$12345678	; illeg
	INC	DWORD #$12345678	; illeg
	INC	DWORD [BX]

	DEC	AL
	DEC	AH
	DEC	BL
	DEC	BH
	DEC	CL
	DEC	CH
	DEC	DL
	DEC	DH
	DEC	#1		; illeg
	DEC	BYTE #1		; illeg
	DEC	[BX]		; illeg
	DEC	BYTE [BX]

	DEC	AX
	DEC	BX
	DEC	CX
	DEC	DX
	DEC	SP
	DEC	BP
	DEC	SI
	DEC	DI
	DEC	CS		; illeg
	DEC	DS		; illeg
	DEC	ES		; illeg
	DEC	FS		; illeg
	DEC	GS		; illeg
	DEC	#$1234		; illeg
	DEC	WORD #$1234	; illeg
	DEC	WORD [BX]

	DEC	EAX
	DEC	EBX
	DEC	ECX
	DEC	EDX
	DEC	ESP
	DEC	EBP
	DEC	ESI
	DEC	EDI
	DEC	#$12345678	; illeg
	DEC	DWORD #$12345678	; illeg
	DEC	DWORD [BX]
