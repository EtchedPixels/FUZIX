	.code
	.setcpu 8080

.export _cpu_identify

_cpu_identify:
	; Ok start with the flags
	mvi a,255
	inr a
	push psw
	pop h
	mov a,l
	ani 0x82
	cpi 0x80
	jz lr35902
	ora a
	jnz is808x
	lxi d,3		; Z80: we don't care which kind.
	ret
lr35902:
	lxi d,4		; also not allowed
	ret
is808x:
	xra a
	.db 0x20	; no-op on 8080 RIM on 8085
	ora a
	jnz is8085	; it changed must be an 8085
	;
	;	But it could really be 0
	;
	inr a
	.db 0x20	; no-op or RIM
	ora a
	jz is8085
	;
	;	TODO: check for KP580M1
	;
	lxi d,0
	;
	;	But wait it might be a 9080
	;
	mvi a,255
	ani 255
	push psw
	pop h
	mov a,l
	ani 0x10	; half carry is zero on AMD
	rnz
	inx d		; 1 = AMD 9080A
	ret
is8085:
	lxi d,2		; 8085
	ret
