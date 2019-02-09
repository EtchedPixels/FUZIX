#

#include "../lib/8080fixedbank-core.s"

!
!	Fast copy 512 bytes from H to D
!
copy512:
	mvi b,64
copy8:
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	mov m,a
	stax d
	inx h
	inx d
	dcr b
	jnz copy8
	ret
