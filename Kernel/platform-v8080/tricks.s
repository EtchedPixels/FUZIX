#

#include "../kernel-8080.def"
#include "../lib/8080fixedbank.s"

.sect .common
!
!	Copy all the user memory from bank a to bank c
!

bankfork:
	lxi d,0x18FE
	lxi h,0
	mov b,a
	! We do D loops of E blocks. 8080 hasn't quite got enough
	! registers to do it in one go so we have to push/pop d thus
	! resetting E each cycle
outer:
	push d
inner:
	! We do 8 bytes per loop and 254 loops per inner loop, so
	! 24 inner loops per run copies the needed space and a tiny shade
	! over (which in this case is fine as it's udata which we will
	! copy from common to common so do nothing to)
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	mov a,b
	out 21
	mov d,m
	mov a,c
	out 21
	mov m,d
	inx h
	dcr e
	jnz inner
	mvi a,'@'
	out 1
	pop d
	dcr d
	jnz outer

	ret
