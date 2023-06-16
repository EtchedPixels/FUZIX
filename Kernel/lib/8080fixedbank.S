#include "../lib/8080fixedbank-core.S"

;
;	Fast copy 512 bytes from H to D
;
copy512:
	mvi b,64
copy8:
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	mov a,m
	stax d
	inx h
	inx d
	dcr b
	jnz copy8
	ret
