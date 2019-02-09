#include "kernel-8080.def"

!
!	Simple implementation for now. Should be optimized
!

.sect .common

.define __uputc

__uputc:
	pop b
	pop d
	pop h
	push h
	push d
	push b
	call map_process_always
	mov m,e
	jp map_kernel

.define __uputw

__uputw:
	pop b
	pop d
	pop h
	push h
	push d
	push b
	call map_process_always
	mov m,e
	inx h
	mov m,d
	jp map_kernel

.define __ugetc

__ugetc:
	pop b
	pop d
	pop h
	push h
	push d
	push b
	call map_process_always
	mov e,m
	jp map_kernel

.define __ugetw

__ugetw:
	pop b
	pop d
	pop h
	push h
	push d
	push b
	call map_process_always
	mov e,m
	inx h
	mov d,m
	jmp map_kernel

.define __uget

!
!	Stacked arguments are src.w, dst.w, count.w
!
__uget:
	push b
	lxi h,9		! End of count argument
	dad sp
	mov b,m
	dcx h
	mov c,m
	mov a,c
	ora b
	jz nowork
	dcx h
	mov d,m		! Destination
	dcx h
	mov e,m
	dcx h
	mov a,m
	dcx h
	mov l,m
	mov h,a
	!
	!	So after all that work we have HL=src DE=dst BC=count
	!	and we know count != 0.
	!
	!	Simple unoptimized copy loop for now. Horribly slow for
	!	things like 512 byte disk blocks
	!
ugetcopy:
	call map_process_always
	mov a,m
	call map_kernel
	stax d
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz ugetcopy
nowork:
	pop b
	ret

.define __uput

__uput:
	push b
	lxi h,9		! End of count argument
	dad sp
	mov b,m
	dcx h
	mov c,m
	mov a,c
	ora b
	jz nowork
	dcx h
	mov d,m		! Destination
	dcx h
	mov e,m
	dcx h
	mov a,m
	dcx h
	mov l,m
	mov h,a
	!
	!	So after all that work we have HL=src DE=dst BC=count
	!	and we know count != 0.
	!
	!	Simple unoptimized copy loop for now. Horribly slow for
	!	things like 512 byte disk blocks
	!
uputcopy:
	mov a,m
	call map_process_always
	stax d
	call map_kernel
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz uputcopy
	pop b
	ret

.define __uzero

__uzero:
	pop d
	pop h
	pop b
	push b
	push h
	push d
	mov a,b
	ora c
	rz
!
!	Simple loop. Wants unrolling a bit
!
	call map_process_always
	xra a
zeroloop:
	mov m,a
	inx h
	dcx b
	mov a,b
	ora c
	jnz zeroloop
	jmp map_kernel

