	MOV	AL,[0]
	MOV	AH,[1]
	MOV	BL,[-1]		; illeg
	MOV	BH,[127]
	MOV	CL,[-128]	; illeg
	MOV	CH,[128]
	MOV	DL,[-129]	; illeg
	MOV	DH,[32767]
	MOV	AL,[-32768]	; illeg
	MOV	AH,[32768]
	MOV	BL,[-32769]	; illeg
	MOV	BH,[$7FFFFFFF]	; illeg
	MOV	CL,[$80000000]	; illeg

	MOV	AL,AL
	MOV	AL,AH
	MOV	AL,BL
	MOV	AL,BH
	MOV	AL,CL
	MOV	AL,CH
	MOV	AL,DL
	MOV	AL,DH

	MOV	AL,AX		; illeg
	MOV	AL,BX		; illeg
	MOV	AL,CX		; illeg
	MOV	AL,DX		; illeg
	MOV	AL,BP		; illeg
	MOV	AL,DI		; illeg
	MOV	AL,SI		; illeg
	MOV	AL,SP		; illeg

	MOV	AH,AL
	MOV	AH,AH
	MOV	AH,BL
	MOV	AH,BH
	MOV	AH,CL
	MOV	AH,CH
	MOV	AH,DL
	MOV	AH,DH

	MOV	AH,AX		; illeg
	MOV	AH,BX		; illeg
	MOV	AH,CX		; illeg
	MOV	AH,DX		; illeg
	MOV	AH,BP		; illeg
	MOV	AH,DI		; illeg
	MOV	AH,SI		; illeg
	MOV	AH,SP		; illeg

	MOV	BL,AL
	MOV	BL,AH
	MOV	BL,BL
	MOV	BL,BH
	MOV	BL,CL
	MOV	BL,CH
	MOV	BL,DL
	MOV	BL,DH

	MOV	BL,AX		; illeg
	MOV	BL,BX		; illeg
	MOV	BL,CX		; illeg
	MOV	BL,DX		; illeg
	MOV	BL,BP		; illeg
	MOV	BL,DI		; illeg
	MOV	BL,SI		; illeg
	MOV	BL,SP		; illeg

	MOV	BH,AL
	MOV	BH,AH
	MOV	BH,BL
	MOV	BH,BH
	MOV	BH,CL
	MOV	BH,CH
	MOV	BH,DL
	MOV	BH,DH

	MOV	BH,AX		; illeg
	MOV	BH,BX		; illeg
	MOV	BH,CX		; illeg
	MOV	BH,DX		; illeg
	MOV	BH,BP		; illeg
	MOV	BH,DI		; illeg
	MOV	BH,SI		; illeg
	MOV	BH,SP		; illeg

	MOV	CL,AL
	MOV	CL,AH
	MOV	CL,BL
	MOV	CL,BH
	MOV	CL,CL
	MOV	CL,CH
	MOV	CL,DL
	MOV	CL,DH

	MOV	CL,AX		; illeg
	MOV	CL,BX		; illeg
	MOV	CL,CX		; illeg
	MOV	CL,DX		; illeg
	MOV	CL,BP		; illeg
	MOV	CL,DI		; illeg
	MOV	CL,SI		; illeg
	MOV	CL,SP		; illeg

	MOV	CH,AL
	MOV	CH,AH
	MOV	CH,BL
	MOV	CH,BH
	MOV	CH,CL
	MOV	CH,CH
	MOV	CH,DL
	MOV	CH,DH

	MOV	CH,AX		; illeg
	MOV	CH,BX		; illeg
	MOV	CH,CX		; illeg
	MOV	CH,DX		; illeg
	MOV	CH,BP		; illeg
	MOV	CH,DI		; illeg
	MOV	CH,SI		; illeg
	MOV	CH,SP		; illeg

	MOV	DL,AL
	MOV	DL,AH
	MOV	DL,BL
	MOV	DL,BH
	MOV	DL,CL
	MOV	DL,CH
	MOV	DL,DL
	MOV	DL,DH

	MOV	DL,AX		; illeg
	MOV	DL,BX		; illeg
	MOV	DL,CX		; illeg
	MOV	DL,DX		; illeg
	MOV	DL,BP		; illeg
	MOV	DL,DI		; illeg
	MOV	DL,SI		; illeg
	MOV	DL,SP		; illeg

	MOV	DH,AL
	MOV	DH,AH
	MOV	DH,BL
	MOV	DH,BH
	MOV	DH,CL
	MOV	DH,CH
	MOV	DH,DL
	MOV	DH,DH

	MOV	DH,AX		; illeg
	MOV	DH,BX		; illeg
	MOV	DH,CX		; illeg
	MOV	DH,DX		; illeg
	MOV	DH,BP		; illeg
	MOV	DH,DI		; illeg
	MOV	DH,SI		; illeg
	MOV	DH,SP		; illeg

	MOV	AL,[AL]		; illeg
	MOV	AH,[AH]		; illeg
	MOV	BL,[BL]		; illeg
	MOV	BH,[BH]		; illeg
	MOV	CL,[CL]		; illeg
	MOV	CH,[CH]		; illeg
	MOV	DL,[DL]		; illeg
	MOV	DH,[DH]		; illeg

	MOV	AL,[AX]		; illeg
	MOV	AH,[BX]
	MOV	BL,[CX]		; illeg
	MOV	BH,[DX]		; illeg
	MOV	CL,[BP]
	MOV	CH,[DI]
	MOV	DL,[SI]
	MOV	DH,[SP]		; illeg

	MOV	AL,[AX+1]	; illeg
	MOV	AH,[BX+1]
	MOV	BL,[CX+1]	; illeg
	MOV	BH,[DX+1]	; illeg
	MOV	CL,[BP+1]
	MOV	CH,[DI+1]
	MOV	DL,[SI+1]
	MOV	DH,[SP+1]	; illeg

	MOV	AL,[AX-1]	; illeg
	MOV	AH,[BX-1]
	MOV	BL,[CX-1]	; illeg
	MOV	BH,[DX-1]	; illeg
	MOV	CL,[BP-1]
	MOV	CH,[DI-1]
	MOV	DL,[SI-1]
	MOV	DH,[SP-1]	; illeg

	MOV	AL,[AX+127]	; illeg
	MOV	AH,[BX+127]
	MOV	BL,[CX+127]	; illeg
	MOV	BH,[DX+127]	; illeg
	MOV	CL,[BP+127]
	MOV	CH,[DI+127]
	MOV	DL,[SI+127]
	MOV	DH,[SP+127]	; illeg

	MOV	AL,[AX-128]	; illeg
	MOV	AH,[BX-128]
	MOV	BL,[CX-128]	; illeg
	MOV	BH,[DX-128]	; illeg
	MOV	CL,[BP-128]
	MOV	CH,[DI-128]
	MOV	DL,[SI-128]
	MOV	DH,[SP-128]	; illeg

	MOV	AL,[AX+128]	; illeg
	MOV	AH,[BX+128]
	MOV	BL,[CX+128]	; illeg
	MOV	BH,[DX+128]	; illeg
	MOV	CL,[BP+128]
	MOV	CH,[DI+128]
	MOV	DL,[SI+128]
	MOV	DH,[SP+128]	; illeg

	MOV	AL,[AX-129]	; illeg
	MOV	AH,[BX-129]
	MOV	BL,[CX-129]	; illeg
	MOV	BH,[DX-129]	; illeg
	MOV	CL,[BP-129]
	MOV	CH,[DI-129]
	MOV	DL,[SI-129]
	MOV	DH,[SP-129]	; illeg

	MOV	AL,[AX+32767]	; illeg
	MOV	AH,[BX+32767]
	MOV	BL,[CX+32767]	; illeg
	MOV	BH,[DX+32767]	; illeg
	MOV	CL,[BP+32767]
	MOV	CH,[DI+32767]
	MOV	DL,[SI+32767]
	MOV	DH,[SP+32767]	; illeg

	MOV	AL,[AX-32768]	; illeg
	MOV	AH,[BX-32768]
	MOV	BL,[CX-32768]	; illeg
	MOV	BH,[DX-32768]	; illeg
	MOV	CL,[BP-32768]
	MOV	CH,[DI-32768]
	MOV	DL,[SI-32768]
	MOV	DH,[SP-32768]	; illeg

	MOV	AL,[AX+32768]	; illeg
	MOV	AH,[BX+32768]
	MOV	BL,[CX+32768]	; illeg
	MOV	BH,[DX+32768]	; illeg
	MOV	CL,[BP+32768]
	MOV	CH,[DI+32768]
	MOV	DL,[SI+32768]
	MOV	DH,[SP+32768]	; illeg

	MOV	AL,[AX-32769]	; illeg
	MOV	AH,[BX-32769]
	MOV	BL,[CX-32769]	; illeg
	MOV	BH,[DX-32769]	; illeg
	MOV	CL,[BP-32769]
	MOV	CH,[DI-32769]
	MOV	DL,[SI-32769]
	MOV	DH,[SP-32769]	; illeg

	MOV	AL,[AX+$7FFFFFFF]	; illeg
	MOV	AH,[BX+$7FFFFFFF]	; illeg (bounds)
	MOV	BL,[CX+$7FFFFFFF]	; illeg
	MOV	BH,[DX+$7FFFFFFF]	; illeg
	MOV	CL,[BP+$7FFFFFFF]	; illeg (bounds)
	MOV	CH,[DI+$7FFFFFFF]	; illeg (bounds)
	MOV	DL,[SI+$7FFFFFFF]	; illeg (bounds)
	MOV	DH,[SP+$7FFFFFFF]	; illeg

	MOV	AL,[AX-$80000000]	; illeg
	MOV	AH,[BX-$80000000]	; illeg (bounds)
	MOV	BL,[CX-$80000000]	; illeg
	MOV	BH,[DX-$80000000]	; illeg
	MOV	CL,[BP-$80000000]	; illeg (bounds)
	MOV	CH,[DI-$80000000]	; illeg (bounds)
	MOV	DL,[SI-$80000000]	; illeg (bounds)
	MOV	DH,[SP-$80000000]	; illeg

	MOV	AL,[AX+AX]	; illeg
	MOV	AH,[AX+BX]	; illeg
	MOV	BL,[AX+CX]	; illeg
	MOV	BH,[AX+DX]	; illeg
	MOV	CL,[AX+BP]	; illeg
	MOV	CH,[AX+DI]	; illeg
	MOV	DL,[AX+SI]	; illeg
	MOV	DH,[AX+SP]	; illeg

	MOV	AL,[BX+AX]	; illeg
	MOV	AH,[BX+BX]	; illeg
	MOV	BL,[BX+CX]	; illeg
	MOV	BH,[BX+DX]	; illeg
	MOV	CL,[BX+BP]	; illeg
	MOV	CH,[BX+DI]
	MOV	DL,[BX+SI]
	MOV	DH,[BX+SP]	; illeg

	MOV	AL,[CX+AX]	; illeg
	MOV	AH,[CX+BX]	; illeg
	MOV	BL,[CX+CX]	; illeg
	MOV	BH,[CX+DX]	; illeg
	MOV	CL,[CX+BP]	; illeg
	MOV	CH,[CX+DI]	; illeg
	MOV	DL,[CX+SI]	; illeg
	MOV	DH,[CX+SP]	; illeg

	MOV	AL,[DX+AX]	; illeg
	MOV	AH,[DX+BX]	; illeg
	MOV	BL,[DX+CX]	; illeg
	MOV	BH,[DX+DX]	; illeg
	MOV	CL,[DX+BP]	; illeg
	MOV	CH,[DX+DI]	; illeg
	MOV	DL,[DX+SI]	; illeg
	MOV	DH,[DX+SP]	; illeg

	MOV	AL,[BP+AX]	; illeg
	MOV	AH,[BP+BX]	; illeg
	MOV	BL,[BP+CX]	; illeg
	MOV	BH,[BP+DX]	; illeg
	MOV	CL,[BP+BP]	; illeg
	MOV	CH,[BP+DI]
	MOV	DL,[BP+SI]
	MOV	DH,[BP+SP]	; illeg

	MOV	AL,[DI+AX]	; illeg
	MOV	AH,[DI+BX]
	MOV	BL,[DI+CX]	; illeg
	MOV	BH,[DI+DX]	; illeg
	MOV	CL,[DI+BP]
	MOV	CH,[DI+DI]	; illeg
	MOV	DL,[DI+SI]	; illeg
	MOV	DH,[DI+SP]	; illeg

	MOV	AL,[SI+AX]	; illeg
	MOV	AH,[SI+BX]
	MOV	BL,[SI+CX]	; illeg
	MOV	BH,[SI+DX]	; illeg
	MOV	CL,[SI+BP]
	MOV	CH,[SI+DI]	; illeg
	MOV	DL,[SI+SI]	; illeg
	MOV	DH,[SI+SP]	; illeg

	MOV	AL,[SP+AX]	; illeg
	MOV	AH,[SP+BX]	; illeg
	MOV	BL,[SP+CX]	; illeg
	MOV	BH,[SP+DX]	; illeg
	MOV	CL,[SP+BP]	; illeg
	MOV	CH,[SP+DI]	; illeg
	MOV	DL,[SP+SI]	; illeg
	MOV	DH,[SP+SP]	; illeg

	MOV	AL,[AX+AX+1]	; illeg
	MOV	AH,[AX+BX+1]	; illeg
	MOV	BL,[AX+CX+1]	; illeg
	MOV	BH,[AX+DX+1]	; illeg
	MOV	CL,[AX+BP+1]	; illeg
	MOV	CH,[AX+DI+1]	; illeg
	MOV	DL,[AX+SI+1]	; illeg
	MOV	DH,[AX+SP+1]	; illeg

	MOV	AL,[BX+AX+1]	; illeg
	MOV	AH,[BX+BX+1]	; illeg
	MOV	BL,[BX+CX+1]	; illeg
	MOV	BH,[BX+DX+1]	; illeg
	MOV	CL,[BX+BP+1]	; illeg
	MOV	CH,[BX+DI+1]
	MOV	DL,[BX+SI+1]
	MOV	DH,[BX+SP+1]	; illeg

	MOV	AL,[CX+AX+1]	; illeg
	MOV	AH,[CX+BX+1]	; illeg
	MOV	BL,[CX+CX+1]	; illeg
	MOV	BH,[CX+DX+1]	; illeg
	MOV	CL,[CX+BP+1]	; illeg
	MOV	CH,[CX+DI+1]	; illeg
	MOV	DL,[CX+SI+1]	; illeg
	MOV	DH,[CX+SP+1]	; illeg

	MOV	AL,[DX+AX+1]	; illeg
	MOV	AH,[DX+BX+1]	; illeg
	MOV	BL,[DX+CX+1]	; illeg
	MOV	BH,[DX+DX+1]	; illeg
	MOV	CL,[DX+BP+1]	; illeg
	MOV	CH,[DX+DI+1]	; illeg
	MOV	DL,[DX+SI+1]	; illeg
	MOV	DH,[DX+SP+1]	; illeg

	MOV	AL,[BP+AX+1]	; illeg
	MOV	AH,[BP+BX+1]	; illeg
	MOV	BL,[BP+CX+1]	; illeg
	MOV	BH,[BP+DX+1]	; illeg
	MOV	CL,[BP+BP+1]	; illeg
	MOV	CH,[BP+DI+1]
	MOV	DL,[BP+SI+1]
	MOV	DH,[BP+SP+1]	; illeg

	MOV	AL,[DI+AX+1]	; illeg
	MOV	AH,[DI+BX+1]
	MOV	BL,[DI+CX+1]	; illeg
	MOV	BH,[DI+DX+1]	; illeg
	MOV	CL,[DI+BP+1]
	MOV	CH,[DI+DI+1]	; illeg
	MOV	DL,[DI+SI+1]	; illeg
	MOV	DH,[DI+SP+1]	; illeg

	MOV	AL,[SI+AX+1]	; illeg
	MOV	AH,[SI+BX+1]
	MOV	BL,[SI+CX+1]	; illeg
	MOV	BH,[SI+DX+1]	; illeg
	MOV	CL,[SI+BP+1]
	MOV	CH,[SI+DI+1]	; illeg
	MOV	DL,[SI+SI+1]	; illeg
	MOV	DH,[SI+SP+1]	; illeg

	MOV	AL,[SP+AX+1]	; illeg
	MOV	AH,[SP+BX+1]	; illeg
	MOV	BL,[SP+CX+1]	; illeg
	MOV	BH,[SP+DX+1]	; illeg
	MOV	CL,[SP+BP+1]	; illeg
	MOV	CH,[SP+DI+1]	; illeg
	MOV	DL,[SP+SI+1]	; illeg
	MOV	DH,[SP+SP+1]	; illeg

	MOV	AL,[AX+AX-1]	; illeg
	MOV	AH,[AX+BX-1]	; illeg
	MOV	BL,[AX+CX-1]	; illeg
	MOV	BH,[AX+DX-1]	; illeg
	MOV	CL,[AX+BP-1]	; illeg
	MOV	CH,[AX+DI-1]	; illeg
	MOV	DL,[AX+SI-1]	; illeg
	MOV	DH,[AX+SP-1]	; illeg

	MOV	AL,[BX+AX-1]	; illeg
	MOV	AH,[BX+BX-1]	; illeg
	MOV	BL,[BX+CX-1]	; illeg
	MOV	BH,[BX+DX-1]	; illeg
	MOV	CL,[BX+BP-1]	; illeg
	MOV	CH,[BX+DI-1]
	MOV	DL,[BX+SI-1]
	MOV	DH,[BX+SP-1]	; illeg

	MOV	AL,[CX+AX-1]	; illeg
	MOV	AH,[CX+BX-1]	; illeg
	MOV	BL,[CX+CX-1]	; illeg
	MOV	BH,[CX+DX-1]	; illeg
	MOV	CL,[CX+BP-1]	; illeg
	MOV	CH,[CX+DI-1]	; illeg
	MOV	DL,[CX+SI-1]	; illeg
	MOV	DH,[CX+SP-1]	; illeg

	MOV	AL,[DX+AX-1]	; illeg
	MOV	AH,[DX+BX-1]	; illeg
	MOV	BL,[DX+CX-1]	; illeg
	MOV	BH,[DX+DX-1]	; illeg
	MOV	CL,[DX+BP-1]	; illeg
	MOV	CH,[DX+DI-1]	; illeg
	MOV	DL,[DX+SI-1]	; illeg
	MOV	DH,[DX+SP-1]	; illeg

	MOV	AL,[BP+AX-1]	; illeg
	MOV	AH,[BP+BX-1]	; illeg
	MOV	BL,[BP+CX-1]	; illeg
	MOV	BH,[BP+DX-1]	; illeg
	MOV	CL,[BP+BP-1]	; illeg
	MOV	CH,[BP+DI-1]
	MOV	DL,[BP+SI-1]
	MOV	DH,[BP+SP-1]	; illeg

	MOV	AL,[DI+AX-1]	; illeg
	MOV	AH,[DI+BX-1]
	MOV	BL,[DI+CX-1]	; illeg
	MOV	BH,[DI+DX-1]	; illeg
	MOV	CL,[DI+BP-1]
	MOV	CH,[DI+DI-1]	; illeg
	MOV	DL,[DI+SI-1]	; illeg
	MOV	DH,[DI+SP-1]	; illeg

	MOV	AL,[SI+AX-1]	; illeg
	MOV	AH,[SI+BX-1]
	MOV	BL,[SI+CX-1]	; illeg
	MOV	BH,[SI+DX-1]	; illeg
	MOV	CL,[SI+BP-1]
	MOV	CH,[SI+DI-1]	; illeg
	MOV	DL,[SI+SI-1]	; illeg
	MOV	DH,[SI+SP-1]	; illeg

	MOV	AL,[SP+AX-1]	; illeg
	MOV	AH,[SP+BX-1]	; illeg
	MOV	BL,[SP+CX-1]	; illeg
	MOV	BH,[SP+DX-1]	; illeg
	MOV	CL,[SP+BP-1]	; illeg
	MOV	CH,[SP+DI-1]	; illeg
	MOV	DL,[SP+SI-1]	; illeg
	MOV	DH,[SP+SP-1]	; illeg

	MOV	AL,[AX+AX+127]	; illeg
	MOV	AH,[AX+BX+127]	; illeg
	MOV	BL,[AX+CX+127]	; illeg
	MOV	BH,[AX+DX+127]	; illeg
	MOV	CL,[AX+BP+127]	; illeg
	MOV	CH,[AX+DI+127]	; illeg
	MOV	DL,[AX+SI+127]	; illeg
	MOV	DH,[AX+SP+127]	; illeg

	MOV	AL,[BX+AX+127]	; illeg
	MOV	AH,[BX+BX+127]	; illeg
	MOV	BL,[BX+CX+127]	; illeg
	MOV	BH,[BX+DX+127]	; illeg
	MOV	CL,[BX+BP+127]	; illeg
	MOV	CH,[BX+DI+127]
	MOV	DL,[BX+SI+127]
	MOV	DH,[BX+SP+127]	; illeg

	MOV	AL,[CX+AX+127]	; illeg
	MOV	AH,[CX+BX+127]	; illeg
	MOV	BL,[CX+CX+127]	; illeg
	MOV	BH,[CX+DX+127]	; illeg
	MOV	CL,[CX+BP+127]	; illeg
	MOV	CH,[CX+DI+127]	; illeg
	MOV	DL,[CX+SI+127]	; illeg
	MOV	DH,[CX+SP+127]	; illeg

	MOV	AL,[DX+AX+127]	; illeg
	MOV	AH,[DX+BX+127]	; illeg
	MOV	BL,[DX+CX+127]	; illeg
	MOV	BH,[DX+DX+127]	; illeg
	MOV	CL,[DX+BP+127]	; illeg
	MOV	CH,[DX+DI+127]	; illeg
	MOV	DL,[DX+SI+127]	; illeg
	MOV	DH,[DX+SP+127]	; illeg

	MOV	AL,[BP+AX+127]	; illeg
	MOV	AH,[BP+BX+127]	; illeg
	MOV	BL,[BP+CX+127]	; illeg
	MOV	BH,[BP+DX+127]	; illeg
	MOV	CL,[BP+BP+127]	; illeg
	MOV	CH,[BP+DI+127]
	MOV	DL,[BP+SI+127]
	MOV	DH,[BP+SP+127]	; illeg

	MOV	AL,[DI+AX+127]	; illeg
	MOV	AH,[DI+BX+127]
	MOV	BL,[DI+CX+127]	; illeg
	MOV	BH,[DI+DX+127]	; illeg
	MOV	CL,[DI+BP+127]
	MOV	CH,[DI+DI+127]	; illeg
	MOV	DL,[DI+SI+127]	; illeg
	MOV	DH,[DI+SP+127]	; illeg

	MOV	AL,[SI+AX+127]	; illeg
	MOV	AH,[SI+BX+127]
	MOV	BL,[SI+CX+127]	; illeg
	MOV	BH,[SI+DX+127]	; illeg
	MOV	CL,[SI+BP+127]
	MOV	CH,[SI+DI+127]	; illeg
	MOV	DL,[SI+SI+127]	; illeg
	MOV	DH,[SI+SP+127]	; illeg

	MOV	AL,[SP+AX+127]	; illeg
	MOV	AH,[SP+BX+127]	; illeg
	MOV	BL,[SP+CX+127]	; illeg
	MOV	BH,[SP+DX+127]	; illeg
	MOV	CL,[SP+BP+127]	; illeg
	MOV	CH,[SP+DI+127]	; illeg
	MOV	DL,[SP+SI+127]	; illeg
	MOV	DH,[SP+SP+127]	; illeg

	MOV	AL,[AX+AX-128]	; illeg
	MOV	AH,[AX+BX-128]	; illeg
	MOV	BL,[AX+CX-128]	; illeg
	MOV	BH,[AX+DX-128]	; illeg
	MOV	CL,[AX+BP-128]	; illeg
	MOV	CH,[AX+DI-128]	; illeg
	MOV	DL,[AX+SI-128]	; illeg
	MOV	DH,[AX+SP-128]	; illeg

	MOV	AL,[BX+AX-128]	; illeg
	MOV	AH,[BX+BX-128]	; illeg
	MOV	BL,[BX+CX-128]	; illeg
	MOV	BH,[BX+DX-128]	; illeg
	MOV	CL,[BX+BP-128]	; illeg
	MOV	CH,[BX+DI-128]
	MOV	DL,[BX+SI-128]
	MOV	DH,[BX+SP-128]	; illeg

	MOV	AL,[CX+AX-128]	; illeg
	MOV	AH,[CX+BX-128]	; illeg
	MOV	BL,[CX+CX-128]	; illeg
	MOV	BH,[CX+DX-128]	; illeg
	MOV	CL,[CX+BP-128]	; illeg
	MOV	CH,[CX+DI-128]	; illeg
	MOV	DL,[CX+SI-128]	; illeg
	MOV	DH,[CX+SP-128]	; illeg

	MOV	AL,[DX+AX-128]	; illeg
	MOV	AH,[DX+BX-128]	; illeg
	MOV	BL,[DX+CX-128]	; illeg
	MOV	BH,[DX+DX-128]	; illeg
	MOV	CL,[DX+BP-128]	; illeg
	MOV	CH,[DX+DI-128]	; illeg
	MOV	DL,[DX+SI-128]	; illeg
	MOV	DH,[DX+SP-128]	; illeg

	MOV	AL,[BP+AX-128]	; illeg
	MOV	AH,[BP+BX-128]	; illeg
	MOV	BL,[BP+CX-128]	; illeg
	MOV	BH,[BP+DX-128]	; illeg
	MOV	CL,[BP+BP-128]	; illeg
	MOV	CH,[BP+DI-128]
	MOV	DL,[BP+SI-128]
	MOV	DH,[BP+SP-128]	; illeg

	MOV	AL,[DI+AX-128]	; illeg
	MOV	AH,[DI+BX-128]
	MOV	BL,[DI+CX-128]	; illeg
	MOV	BH,[DI+DX-128]	; illeg
	MOV	CL,[DI+BP-128]
	MOV	CH,[DI+DI-128]	; illeg
	MOV	DL,[DI+SI-128]	; illeg
	MOV	DH,[DI+SP-128]	; illeg

	MOV	AL,[SI+AX-128]	; illeg
	MOV	AH,[SI+BX-128]
	MOV	BL,[SI+CX-128]	; illeg
	MOV	BH,[SI+DX-128]	; illeg
	MOV	CL,[SI+BP-128]
	MOV	CH,[SI+DI-128]	; illeg
	MOV	DL,[SI+SI-128]	; illeg
	MOV	DH,[SI+SP-128]	; illeg

	MOV	AL,[SP+AX-128]	; illeg
	MOV	AH,[SP+BX-128]	; illeg
	MOV	BL,[SP+CX-128]	; illeg
	MOV	BH,[SP+DX-128]	; illeg
	MOV	CL,[SP+BP-128]	; illeg
	MOV	CH,[SP+DI-128]	; illeg
	MOV	DL,[SP+SI-128]	; illeg
	MOV	DH,[SP+SP-128]	; illeg

	MOV	AL,[AX+AX+128]	; illeg
	MOV	AH,[AX+BX+128]	; illeg
	MOV	BL,[AX+CX+128]	; illeg
	MOV	BH,[AX+DX+128]	; illeg
	MOV	CL,[AX+BP+128]	; illeg
	MOV	CH,[AX+DI+128]	; illeg
	MOV	DL,[AX+SI+128]	; illeg
	MOV	DH,[AX+SP+128]	; illeg

	MOV	AL,[BX+AX+128]	; illeg
	MOV	AH,[BX+BX+128]	; illeg
	MOV	BL,[BX+CX+128]	; illeg
	MOV	BH,[BX+DX+128]	; illeg
	MOV	CL,[BX+BP+128]	; illeg
	MOV	CH,[BX+DI+128]
	MOV	DL,[BX+SI+128]
	MOV	DH,[BX+SP+128]	; illeg

	MOV	AL,[CX+AX+128]	; illeg
	MOV	AH,[CX+BX+128]	; illeg
	MOV	BL,[CX+CX+128]	; illeg
	MOV	BH,[CX+DX+128]	; illeg
	MOV	CL,[CX+BP+128]	; illeg
	MOV	CH,[CX+DI+128]	; illeg
	MOV	DL,[CX+SI+128]	; illeg
	MOV	DH,[CX+SP+128]	; illeg

	MOV	AL,[DX+AX+128]	; illeg
	MOV	AH,[DX+BX+128]	; illeg
	MOV	BL,[DX+CX+128]	; illeg
	MOV	BH,[DX+DX+128]	; illeg
	MOV	CL,[DX+BP+128]	; illeg
	MOV	CH,[DX+DI+128]	; illeg
	MOV	DL,[DX+SI+128]	; illeg
	MOV	DH,[DX+SP+128]	; illeg

	MOV	AL,[BP+AX+128]	; illeg
	MOV	AH,[BP+BX+128]	; illeg
	MOV	BL,[BP+CX+128]	; illeg
	MOV	BH,[BP+DX+128]	; illeg
	MOV	CL,[BP+BP+128]	; illeg
	MOV	CH,[BP+DI+128]
	MOV	DL,[BP+SI+128]
	MOV	DH,[BP+SP+128]	; illeg

	MOV	AL,[DI+AX+128]	; illeg
	MOV	AH,[DI+BX+128]
	MOV	BL,[DI+CX+128]	; illeg
	MOV	BH,[DI+DX+128]	; illeg
	MOV	CL,[DI+BP+128]
	MOV	CH,[DI+DI+128]	; illeg
	MOV	DL,[DI+SI+128]	; illeg
	MOV	DH,[DI+SP+128]	; illeg

	MOV	AL,[SI+AX+128]	; illeg
	MOV	AH,[SI+BX+128]
	MOV	BL,[SI+CX+128]	; illeg
	MOV	BH,[SI+DX+128]	; illeg
	MOV	CL,[SI+BP+128]
	MOV	CH,[SI+DI+128]	; illeg
	MOV	DL,[SI+SI+128]	; illeg
	MOV	DH,[SI+SP+128]	; illeg

	MOV	AL,[SP+AX+128]	; illeg
	MOV	AH,[SP+BX+128]	; illeg
	MOV	BL,[SP+CX+128]	; illeg
	MOV	BH,[SP+DX+128]	; illeg
	MOV	CL,[SP+BP+128]	; illeg
	MOV	CH,[SP+DI+128]	; illeg
	MOV	DL,[SP+SI+128]	; illeg
	MOV	DH,[SP+SP+128]	; illeg

	MOV	AL,[AX+AX-129]	; illeg
	MOV	AH,[AX+BX-129]	; illeg
	MOV	BL,[AX+CX-129]	; illeg
	MOV	BH,[AX+DX-129]	; illeg
	MOV	CL,[AX+BP-129]	; illeg
	MOV	CH,[AX+DI-129]	; illeg
	MOV	DL,[AX+SI-129]	; illeg
	MOV	DH,[AX+SP-129]	; illeg

	MOV	AL,[BX+AX-129]	; illeg
	MOV	AH,[BX+BX-129]	; illeg
	MOV	BL,[BX+CX-129]	; illeg
	MOV	BH,[BX+DX-129]	; illeg
	MOV	CL,[BX+BP-129]	; illeg
	MOV	CH,[BX+DI-129]
	MOV	DL,[BX+SI-129]
	MOV	DH,[BX+SP-129]	; illeg

	MOV	AL,[CX+AX-129]	; illeg
	MOV	AH,[CX+BX-129]	; illeg
	MOV	BL,[CX+CX-129]	; illeg
	MOV	BH,[CX+DX-129]	; illeg
	MOV	CL,[CX+BP-129]	; illeg
	MOV	CH,[CX+DI-129]	; illeg
	MOV	DL,[CX+SI-129]	; illeg
	MOV	DH,[CX+SP-129]	; illeg

	MOV	AL,[DX+AX-129]	; illeg
	MOV	AH,[DX+BX-129]	; illeg
	MOV	BL,[DX+CX-129]	; illeg
	MOV	BH,[DX+DX-129]	; illeg
	MOV	CL,[DX+BP-129]	; illeg
	MOV	CH,[DX+DI-129]	; illeg
	MOV	DL,[DX+SI-129]	; illeg
	MOV	DH,[DX+SP-129]	; illeg

	MOV	AL,[BP+AX-129]	; illeg
	MOV	AH,[BP+BX-129]	; illeg
	MOV	BL,[BP+CX-129]	; illeg
	MOV	BH,[BP+DX-129]	; illeg
	MOV	CL,[BP+BP-129]	; illeg
	MOV	CH,[BP+DI-129]
	MOV	DL,[BP+SI-129]
	MOV	DH,[BP+SP-129]	; illeg

	MOV	AL,[DI+AX-129]	; illeg
	MOV	AH,[DI+BX-129]
	MOV	BL,[DI+CX-129]	; illeg
	MOV	BH,[DI+DX-129]	; illeg
	MOV	CL,[DI+BP-129]
	MOV	CH,[DI+DI-129]	; illeg
	MOV	DL,[DI+SI-129]	; illeg
	MOV	DH,[DI+SP-129]	; illeg

	MOV	AL,[SI+AX-129]	; illeg
	MOV	AH,[SI+BX-129]
	MOV	BL,[SI+CX-129]	; illeg
	MOV	BH,[SI+DX-129]	; illeg
	MOV	CL,[SI+BP-129]
	MOV	CH,[SI+DI-129]	; illeg
	MOV	DL,[SI+SI-129]	; illeg
	MOV	DH,[SI+SP-129]	; illeg

	MOV	AL,[SP+AX-129]	; illeg
	MOV	AH,[SP+BX-129]	; illeg
	MOV	BL,[SP+CX-129]	; illeg
	MOV	BH,[SP+DX-129]	; illeg
	MOV	CL,[SP+BP-129]	; illeg
	MOV	CH,[SP+DI-129]	; illeg
	MOV	DL,[SP+SI-129]	; illeg
	MOV	DH,[SP+SP-129]	; illeg

	MOV	AL,[AX+AX+32767]	; illeg
	MOV	AH,[AX+BX+32767]	; illeg
	MOV	BL,[AX+CX+32767]	; illeg
	MOV	BH,[AX+DX+32767]	; illeg
	MOV	CL,[AX+BP+32767]	; illeg
	MOV	CH,[AX+DI+32767]	; illeg
	MOV	DL,[AX+SI+32767]	; illeg
	MOV	DH,[AX+SP+32767]	; illeg

	MOV	AL,[BX+AX+32767]	; illeg
	MOV	AH,[BX+BX+32767]	; illeg
	MOV	BL,[BX+CX+32767]	; illeg
	MOV	BH,[BX+DX+32767]	; illeg
	MOV	CL,[BX+BP+32767]	; illeg
	MOV	CH,[BX+DI+32767]
	MOV	DL,[BX+SI+32767]
	MOV	DH,[BX+SP+32767]	; illeg

	MOV	AL,[CX+AX+32767]	; illeg
	MOV	AH,[CX+BX+32767]	; illeg
	MOV	BL,[CX+CX+32767]	; illeg
	MOV	BH,[CX+DX+32767]	; illeg
	MOV	CL,[CX+BP+32767]	; illeg
	MOV	CH,[CX+DI+32767]	; illeg
	MOV	DL,[CX+SI+32767]	; illeg
	MOV	DH,[CX+SP+32767]	; illeg

	MOV	AL,[DX+AX+32767]	; illeg
	MOV	AH,[DX+BX+32767]	; illeg
	MOV	BL,[DX+CX+32767]	; illeg
	MOV	BH,[DX+DX+32767]	; illeg
	MOV	CL,[DX+BP+32767]	; illeg
	MOV	CH,[DX+DI+32767]	; illeg
	MOV	DL,[DX+SI+32767]	; illeg
	MOV	DH,[DX+SP+32767]	; illeg

	MOV	AL,[BP+AX+32767]	; illeg
	MOV	AH,[BP+BX+32767]	; illeg
	MOV	BL,[BP+CX+32767]	; illeg
	MOV	BH,[BP+DX+32767]	; illeg
	MOV	CL,[BP+BP+32767]	; illeg
	MOV	CH,[BP+DI+32767]
	MOV	DL,[BP+SI+32767]
	MOV	DH,[BP+SP+32767]	; illeg

	MOV	AL,[DI+AX+32767]	; illeg
	MOV	AH,[DI+BX+32767]
	MOV	BL,[DI+CX+32767]	; illeg
	MOV	BH,[DI+DX+32767]	; illeg
	MOV	CL,[DI+BP+32767]
	MOV	CH,[DI+DI+32767]	; illeg
	MOV	DL,[DI+SI+32767]	; illeg
	MOV	DH,[DI+SP+32767]	; illeg

	MOV	AL,[SI+AX+32767]	; illeg
	MOV	AH,[SI+BX+32767]
	MOV	BL,[SI+CX+32767]	; illeg
	MOV	BH,[SI+DX+32767]	; illeg
	MOV	CL,[SI+BP+32767]
	MOV	CH,[SI+DI+32767]	; illeg
	MOV	DL,[SI+SI+32767]	; illeg
	MOV	DH,[SI+SP+32767]	; illeg

	MOV	AL,[SP+AX+32767]	; illeg
	MOV	AH,[SP+BX+32767]	; illeg
	MOV	BL,[SP+CX+32767]	; illeg
	MOV	BH,[SP+DX+32767]	; illeg
	MOV	CL,[SP+BP+32767]	; illeg
	MOV	CH,[SP+DI+32767]	; illeg
	MOV	DL,[SP+SI+32767]	; illeg
	MOV	DH,[SP+SP+32767]	; illeg

	MOV	AL,[AX+AX-32768]	; illeg
	MOV	AH,[AX+BX-32768]	; illeg
	MOV	BL,[AX+CX-32768]	; illeg
	MOV	BH,[AX+DX-32768]	; illeg
	MOV	CL,[AX+BP-32768]	; illeg
	MOV	CH,[AX+DI-32768]	; illeg
	MOV	DL,[AX+SI-32768]	; illeg
	MOV	DH,[AX+SP-32768]	; illeg

	MOV	AL,[BX+AX-32768]	; illeg
	MOV	AH,[BX+BX-32768]	; illeg
	MOV	BL,[BX+CX-32768]	; illeg
	MOV	BH,[BX+DX-32768]	; illeg
	MOV	CL,[BX+BP-32768]	; illeg
	MOV	CH,[BX+DI-32768]
	MOV	DL,[BX+SI-32768]
	MOV	DH,[BX+SP-32768]	; illeg

	MOV	AL,[CX+AX-32768]	; illeg
	MOV	AH,[CX+BX-32768]	; illeg
	MOV	BL,[CX+CX-32768]	; illeg
	MOV	BH,[CX+DX-32768]	; illeg
	MOV	CL,[CX+BP-32768]	; illeg
	MOV	CH,[CX+DI-32768]	; illeg
	MOV	DL,[CX+SI-32768]	; illeg
	MOV	DH,[CX+SP-32768]	; illeg

	MOV	AL,[DX+AX-32768]	; illeg
	MOV	AH,[DX+BX-32768]	; illeg
	MOV	BL,[DX+CX-32768]	; illeg
	MOV	BH,[DX+DX-32768]	; illeg
	MOV	CL,[DX+BP-32768]	; illeg
	MOV	CH,[DX+DI-32768]	; illeg
	MOV	DL,[DX+SI-32768]	; illeg
	MOV	DH,[DX+SP-32768]	; illeg

	MOV	AL,[BP+AX-32768]	; illeg
	MOV	AH,[BP+BX-32768]	; illeg
	MOV	BL,[BP+CX-32768]	; illeg
	MOV	BH,[BP+DX-32768]	; illeg
	MOV	CL,[BP+BP-32768]	; illeg
	MOV	CH,[BP+DI-32768]
	MOV	DL,[BP+SI-32768]
	MOV	DH,[BP+SP-32768]	; illeg

	MOV	AL,[DI+AX-32768]	; illeg
	MOV	AH,[DI+BX-32768]
	MOV	BL,[DI+CX-32768]	; illeg
	MOV	BH,[DI+DX-32768]	; illeg
	MOV	CL,[DI+BP-32768]
	MOV	CH,[DI+DI-32768]	; illeg
	MOV	DL,[DI+SI-32768]	; illeg
	MOV	DH,[DI+SP-32768]	; illeg

	MOV	AL,[SI+AX-32768]	; illeg
	MOV	AH,[SI+BX-32768]
	MOV	BL,[SI+CX-32768]	; illeg
	MOV	BH,[SI+DX-32768]	; illeg
	MOV	CL,[SI+BP-32768]
	MOV	CH,[SI+DI-32768]	; illeg
	MOV	DL,[SI+SI-32768]	; illeg
	MOV	DH,[SI+SP-32768]	; illeg

	MOV	AL,[SP+AX-32768]	; illeg
	MOV	AH,[SP+BX-32768]	; illeg
	MOV	BL,[SP+CX-32768]	; illeg
	MOV	BH,[SP+DX-32768]	; illeg
	MOV	CL,[SP+BP-32768]	; illeg
	MOV	CH,[SP+DI-32768]	; illeg
	MOV	DL,[SP+SI-32768]	; illeg
	MOV	DH,[SP+SP-32768]	; illeg

	MOV	AL,[AX+AX+32768]	; illeg
	MOV	AH,[AX+BX+32768]	; illeg
	MOV	BL,[AX+CX+32768]	; illeg
	MOV	BH,[AX+DX+32768]	; illeg
	MOV	CL,[AX+BP+32768]	; illeg
	MOV	CH,[AX+DI+32768]	; illeg
	MOV	DL,[AX+SI+32768]	; illeg
	MOV	DH,[AX+SP+32768]	; illeg

	MOV	AL,[BX+AX+32768]	; illeg
	MOV	AH,[BX+BX+32768]	; illeg
	MOV	BL,[BX+CX+32768]	; illeg
	MOV	BH,[BX+DX+32768]	; illeg
	MOV	CL,[BX+BP+32768]	; illeg
	MOV	CH,[BX+DI+32768]
	MOV	DL,[BX+SI+32768]
	MOV	DH,[BX+SP+32768]	; illeg

	MOV	AL,[CX+AX+32768]	; illeg
	MOV	AH,[CX+BX+32768]	; illeg
	MOV	BL,[CX+CX+32768]	; illeg
	MOV	BH,[CX+DX+32768]	; illeg
	MOV	CL,[CX+BP+32768]	; illeg
	MOV	CH,[CX+DI+32768]	; illeg
	MOV	DL,[CX+SI+32768]	; illeg
	MOV	DH,[CX+SP+32768]	; illeg

	MOV	AL,[DX+AX+32768]	; illeg
	MOV	AH,[DX+BX+32768]	; illeg
	MOV	BL,[DX+CX+32768]	; illeg
	MOV	BH,[DX+DX+32768]	; illeg
	MOV	CL,[DX+BP+32768]	; illeg
	MOV	CH,[DX+DI+32768]	; illeg
	MOV	DL,[DX+SI+32768]	; illeg
	MOV	DH,[DX+SP+32768]	; illeg

	MOV	AL,[BP+AX+32768]	; illeg
	MOV	AH,[BP+BX+32768]	; illeg
	MOV	BL,[BP+CX+32768]	; illeg
	MOV	BH,[BP+DX+32768]	; illeg
	MOV	CL,[BP+BP+32768]	; illeg
	MOV	CH,[BP+DI+32768]
	MOV	DL,[BP+SI+32768]
	MOV	DH,[BP+SP+32768]	; illeg

	MOV	AL,[DI+AX+32768]	; illeg
	MOV	AH,[DI+BX+32768]
	MOV	BL,[DI+CX+32768]	; illeg
	MOV	BH,[DI+DX+32768]	; illeg
	MOV	CL,[DI+BP+32768]
	MOV	CH,[DI+DI+32768]	; illeg
	MOV	DL,[DI+SI+32768]	; illeg
	MOV	DH,[DI+SP+32768]	; illeg

	MOV	AL,[SI+AX+32768]	; illeg
	MOV	AH,[SI+BX+32768]
	MOV	BL,[SI+CX+32768]	; illeg
	MOV	BH,[SI+DX+32768]	; illeg
	MOV	CL,[SI+BP+32768]
	MOV	CH,[SI+DI+32768]	; illeg
	MOV	DL,[SI+SI+32768]	; illeg
	MOV	DH,[SI+SP+32768]	; illeg

	MOV	AL,[SP+AX+32768]	; illeg
	MOV	AH,[SP+BX+32768]	; illeg
	MOV	BL,[SP+CX+32768]	; illeg
	MOV	BH,[SP+DX+32768]	; illeg
	MOV	CL,[SP+BP+32768]	; illeg
	MOV	CH,[SP+DI+32768]	; illeg
	MOV	DL,[SP+SI+32768]	; illeg
	MOV	DH,[SP+SP+32768]	; illeg

	MOV	AL,[AX+AX-32769]	; illeg
	MOV	AH,[AX+BX-32769]	; illeg
	MOV	BL,[AX+CX-32769]	; illeg
	MOV	BH,[AX+DX-32769]	; illeg
	MOV	CL,[AX+BP-32769]	; illeg
	MOV	CH,[AX+DI-32769]	; illeg
	MOV	DL,[AX+SI-32769]	; illeg
	MOV	DH,[AX+SP-32769]	; illeg

	MOV	AL,[BX+AX-32769]	; illeg
	MOV	AH,[BX+BX-32769]	; illeg
	MOV	BL,[BX+CX-32769]	; illeg
	MOV	BH,[BX+DX-32769]	; illeg
	MOV	CL,[BX+BP-32769]	; illeg
	MOV	CH,[BX+DI-32769]
	MOV	DL,[BX+SI-32769]
	MOV	DH,[BX+SP-32769]	; illeg

	MOV	AL,[CX+AX-32769]	; illeg
	MOV	AH,[CX+BX-32769]	; illeg
	MOV	BL,[CX+CX-32769]	; illeg
	MOV	BH,[CX+DX-32769]	; illeg
	MOV	CL,[CX+BP-32769]	; illeg
	MOV	CH,[CX+DI-32769]	; illeg
	MOV	DL,[CX+SI-32769]	; illeg
	MOV	DH,[CX+SP-32769]	; illeg

	MOV	AL,[DX+AX-32769]	; illeg
	MOV	AH,[DX+BX-32769]	; illeg
	MOV	BL,[DX+CX-32769]	; illeg
	MOV	BH,[DX+DX-32769]	; illeg
	MOV	CL,[DX+BP-32769]	; illeg
	MOV	CH,[DX+DI-32769]	; illeg
	MOV	DL,[DX+SI-32769]	; illeg
	MOV	DH,[DX+SP-32769]	; illeg

	MOV	AL,[BP+AX-32769]	; illeg
	MOV	AH,[BP+BX-32769]	; illeg
	MOV	BL,[BP+CX-32769]	; illeg
	MOV	BH,[BP+DX-32769]	; illeg
	MOV	CL,[BP+BP-32769]	; illeg
	MOV	CH,[BP+DI-32769]
	MOV	DL,[BP+SI-32769]
	MOV	DH,[BP+SP-32769]	; illeg

	MOV	AL,[DI+AX-32769]	; illeg
	MOV	AH,[DI+BX-32769]
	MOV	BL,[DI+CX-32769]	; illeg
	MOV	BH,[DI+DX-32769]	; illeg
	MOV	CL,[DI+BP-32769]
	MOV	CH,[DI+DI-32769]	; illeg
	MOV	DL,[DI+SI-32769]	; illeg
	MOV	DH,[DI+SP-32769]	; illeg

	MOV	AL,[SI+AX-32769]	; illeg
	MOV	AH,[SI+BX-32769]
	MOV	BL,[SI+CX-32769]	; illeg
	MOV	BH,[SI+DX-32769]	; illeg
	MOV	CL,[SI+BP-32769]
	MOV	CH,[SI+DI-32769]	; illeg
	MOV	DL,[SI+SI-32769]	; illeg
	MOV	DH,[SI+SP-32769]	; illeg

	MOV	AL,[SP+AX-32769]	; illeg
	MOV	AH,[SP+BX-32769]	; illeg
	MOV	BL,[SP+CX-32769]	; illeg
	MOV	BH,[SP+DX-32769]	; illeg
	MOV	CL,[SP+BP-32769]	; illeg
	MOV	CH,[SP+DI-32769]	; illeg
	MOV	DL,[SP+SI-32769]	; illeg
	MOV	DH,[SP+SP-32769]	; illeg

	MOV	AL,[AX+AX+$7FFFFFFF]	; illeg
	MOV	AH,[AX+BX+$7FFFFFFF]	; illeg
	MOV	BL,[AX+CX+$7FFFFFFF]	; illeg
	MOV	BH,[AX+DX+$7FFFFFFF]	; illeg
	MOV	CL,[AX+BP+$7FFFFFFF]	; illeg
	MOV	CH,[AX+DI+$7FFFFFFF]	; illeg
	MOV	DL,[AX+SI+$7FFFFFFF]	; illeg
	MOV	DH,[AX+SP+$7FFFFFFF]	; illeg

	MOV	AL,[BX+AX+$7FFFFFFF]	; illeg
	MOV	AH,[BX+BX+$7FFFFFFF]	; illeg
	MOV	BL,[BX+CX+$7FFFFFFF]	; illeg
	MOV	BH,[BX+DX+$7FFFFFFF]	; illeg
	MOV	CL,[BX+BP+$7FFFFFFF]	; illeg
	MOV	CH,[BX+DI+$7FFFFFFF]	; illeg (bounds)
	MOV	DL,[BX+SI+$7FFFFFFF]	; illeg (bounds)
	MOV	DH,[BX+SP+$7FFFFFFF]	; illeg

	MOV	AL,[CX+AX+$7FFFFFFF]	; illeg
	MOV	AH,[CX+BX+$7FFFFFFF]	; illeg
	MOV	BL,[CX+CX+$7FFFFFFF]	; illeg
	MOV	BH,[CX+DX+$7FFFFFFF]	; illeg
	MOV	CL,[CX+BP+$7FFFFFFF]	; illeg
	MOV	CH,[CX+DI+$7FFFFFFF]	; illeg
	MOV	DL,[CX+SI+$7FFFFFFF]	; illeg
	MOV	DH,[CX+SP+$7FFFFFFF]	; illeg

	MOV	AL,[DX+AX+$7FFFFFFF]	; illeg
	MOV	AH,[DX+BX+$7FFFFFFF]	; illeg
	MOV	BL,[DX+CX+$7FFFFFFF]	; illeg
	MOV	BH,[DX+DX+$7FFFFFFF]	; illeg
	MOV	CL,[DX+BP+$7FFFFFFF]	; illeg
	MOV	CH,[DX+DI+$7FFFFFFF]	; illeg
	MOV	DL,[DX+SI+$7FFFFFFF]	; illeg
	MOV	DH,[DX+SP+$7FFFFFFF]	; illeg

	MOV	AL,[BP+AX+$7FFFFFFF]	; illeg
	MOV	AH,[BP+BX+$7FFFFFFF]	; illeg
	MOV	BL,[BP+CX+$7FFFFFFF]	; illeg
	MOV	BH,[BP+DX+$7FFFFFFF]	; illeg
	MOV	CL,[BP+BP+$7FFFFFFF]	; illeg
	MOV	CH,[BP+DI+$7FFFFFFF]	; illeg (bounds)
	MOV	DL,[BP+SI+$7FFFFFFF]	; illeg (bounds)
	MOV	DH,[BP+SP+$7FFFFFFF]	; illeg

	MOV	AL,[DI+AX+$7FFFFFFF]	; illeg
	MOV	AH,[DI+BX+$7FFFFFFF]	; illeg (bounds)
	MOV	BL,[DI+CX+$7FFFFFFF]	; illeg
	MOV	BH,[DI+DX+$7FFFFFFF]	; illeg
	MOV	CL,[DI+BP+$7FFFFFFF]	; illeg (bounds)
	MOV	CH,[DI+DI+$7FFFFFFF]	; illeg
	MOV	DL,[DI+SI+$7FFFFFFF]	; illeg
	MOV	DH,[DI+SP+$7FFFFFFF]	; illeg

	MOV	AL,[SI+AX+$7FFFFFFF]	; illeg
	MOV	AH,[SI+BX+$7FFFFFFF]	; illeg (bounds)
	MOV	BL,[SI+CX+$7FFFFFFF]	; illeg
	MOV	BH,[SI+DX+$7FFFFFFF]	; illeg
	MOV	CL,[SI+BP+$7FFFFFFF]	; illeg (bounds)
	MOV	CH,[SI+DI+$7FFFFFFF]	; illeg
	MOV	DL,[SI+SI+$7FFFFFFF]	; illeg
	MOV	DH,[SI+SP+$7FFFFFFF]	; illeg

	MOV	AL,[SP+AX+$7FFFFFFF]	; illeg
	MOV	AH,[SP+BX+$7FFFFFFF]	; illeg
	MOV	BL,[SP+CX+$7FFFFFFF]	; illeg
	MOV	BH,[SP+DX+$7FFFFFFF]	; illeg
	MOV	CL,[SP+BP+$7FFFFFFF]	; illeg
	MOV	CH,[SP+DI+$7FFFFFFF]	; illeg
	MOV	DL,[SP+SI+$7FFFFFFF]	; illeg
	MOV	DH,[SP+SP+$7FFFFFFF]	; illeg

	MOV	AL,[AX+AX-$80000000]	; illeg
	MOV	AH,[AX+BX-$80000000]	; illeg
	MOV	BL,[AX+CX-$80000000]	; illeg
	MOV	BH,[AX+DX-$80000000]	; illeg
	MOV	CL,[AX+BP-$80000000]	; illeg
	MOV	CH,[AX+DI-$80000000]	; illeg
	MOV	DL,[AX+SI-$80000000]	; illeg
	MOV	DH,[AX+SP-$80000000]	; illeg

	MOV	AL,[BX+AX-$80000000]	; illeg
	MOV	AH,[BX+BX-$80000000]	; illeg
	MOV	BL,[BX+CX-$80000000]	; illeg
	MOV	BH,[BX+DX-$80000000]	; illeg
	MOV	CL,[BX+BP-$80000000]	; illeg
	MOV	CH,[BX+DI-$80000000]	; illeg (bounds)
	MOV	DL,[BX+SI-$80000000]	; illeg (bounds)
	MOV	DH,[BX+SP-$80000000]	; illeg

	MOV	AL,[CX+AX-$80000000]	; illeg
	MOV	AH,[CX+BX-$80000000]	; illeg
	MOV	BL,[CX+CX-$80000000]	; illeg
	MOV	BH,[CX+DX-$80000000]	; illeg
	MOV	CL,[CX+BP-$80000000]	; illeg
	MOV	CH,[CX+DI-$80000000]	; illeg
	MOV	DL,[CX+SI-$80000000]	; illeg
	MOV	DH,[CX+SP-$80000000]	; illeg

	MOV	AL,[DX+AX-$80000000]	; illeg
	MOV	AH,[DX+BX-$80000000]	; illeg
	MOV	BL,[DX+CX-$80000000]	; illeg
	MOV	BH,[DX+DX-$80000000]	; illeg
	MOV	CL,[DX+BP-$80000000]	; illeg
	MOV	CH,[DX+DI-$80000000]	; illeg
	MOV	DL,[DX+SI-$80000000]	; illeg
	MOV	DH,[DX+SP-$80000000]	; illeg

	MOV	AL,[BP+AX-$80000000]	; illeg
	MOV	AH,[BP+BX-$80000000]	; illeg
	MOV	BL,[BP+CX-$80000000]	; illeg
	MOV	BH,[BP+DX-$80000000]	; illeg
	MOV	CL,[BP+BP-$80000000]	; illeg
	MOV	CH,[BP+DI-$80000000]	; illeg (bounds)
	MOV	DL,[BP+SI-$80000000]	; illeg (bounds)
	MOV	DH,[BP+SP-$80000000]	; illeg

	MOV	AL,[DI+AX-$80000000]	; illeg
	MOV	AH,[DI+BX-$80000000]	; illeg (bounds)
	MOV	BL,[DI+CX-$80000000]	; illeg
	MOV	BH,[DI+DX-$80000000]	; illeg
	MOV	CL,[DI+BP-$80000000]	; illeg (bounds)
	MOV	CH,[DI+DI-$80000000]	; illeg
	MOV	DL,[DI+SI-$80000000]	; illeg
	MOV	DH,[DI+SP-$80000000]	; illeg

	MOV	AL,[SI+AX-$80000000]	; illeg
	MOV	AH,[SI+BX-$80000000]	; illeg (bounds)
	MOV	BL,[SI+CX-$80000000]	; illeg
	MOV	BH,[SI+DX-$80000000]	; illeg
	MOV	CL,[SI+BP-$80000000]	; illeg (bounds)
	MOV	CH,[SI+DI-$80000000]	; illeg
	MOV	DL,[SI+SI-$80000000]	; illeg
	MOV	DH,[SI+SP-$80000000]	; illeg

	MOV	AL,[SP+AX-$80000000]	; illeg
	MOV	AH,[SP+BX-$80000000]	; illeg
	MOV	BL,[SP+CX-$80000000]	; illeg
	MOV	BH,[SP+DX-$80000000]	; illeg
	MOV	CL,[SP+BP-$80000000]	; illeg
	MOV	CH,[SP+DI-$80000000]	; illeg
	MOV	DL,[SP+SI-$80000000]	; illeg
	MOV	DH,[SP+SP-$80000000]	; illeg
