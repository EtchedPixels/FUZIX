#

#include "../lib/8080fixedbank-core.s"

!
!	Fast copy 512 bytes from H to D
!
copy512:
	in 21
	adi 48
	out 1
	mvi a,":"
	out 1
	call outhl
	xchg
	mvi a,"-"
	out 1
	call outhl
	xchg
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
	mvi a,'='
	out 1
	lhld 0xe800
	call outhl
	ret
