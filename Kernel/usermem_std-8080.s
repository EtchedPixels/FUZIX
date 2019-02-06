!
!	Simple implementation for now. Should be optimized
!

.sect .commonmem

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
	mov l,m
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
	mov a,m
	inx h
	mov h,m
	mov l,a
	jmp map_kernel

.define __uget

__uget:
	! TODO
	ret

.define __uput

__uput:
	! TODO
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
