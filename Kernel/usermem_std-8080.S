#include "kernel-8080.def"

;
;	Simple implementation for now. Should be optimized
;

	.common

.export __uputc

__uputc:
	lxi h,2
	dad sp
	mov a,m
	inx h
	inx h
	mov e,m
	inx h
	mov d,m
	call map_proc_always
	stax d
	lxi h,0
	jmp map_kernel

.export __uputw

__uputw:
	lxi h,2
	dad sp
	mov e,m
	inx h
	mov d,m
	inx h
	mov a,m
	inx h
	mov h,m
	mov l,a
	call map_proc_always
	mov m,e
	inx h
	mov m,d
	lxi h,0
	jmp map_kernel

.export __ugetc

__ugetc:
	pop d
	pop h
	push h
	push d
	call map_proc_always
	mov l,m
	mvi h,0
	jmp map_kernel

.export __ugetw

__ugetw:
	pop d
	pop h
	push h
	push d
	call map_proc_always
	mov a,m
	inx h
	mov h,m
	mov l,a
	jmp map_kernel

.export __uget

;
;	Stacked arguments are src.w, dst.w, count.w
;
__uget:
	push b
	lxi h,9		; End of count argument
	dad sp
	mov b,m
	dcx h
	mov c,m
	mov a,c
	ora b
	jz nowork
	dcx h
	mov d,m		; Destination
	dcx h
	mov e,m
	dcx h
	mov a,m
	dcx h
	mov l,m
	mov h,a
	;
	;	So after all that work we have HL=src DE=dst BC=count
	;	and we know count ;= 0.
	;
	;	Simple unoptimized copy loop for now. Horribly slow for
	;	things like 512 byte disk blocks
	;
ugetcopy:
	call map_proc_always
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
	lxi h,0
	ret

.export __uput

__uput:
	push b
	lxi h,9		; End of count argument
	dad sp
	mov b,m
	dcx h
	mov c,m
	mov a,c
	ora b
	jz nowork
	dcx h
	mov d,m		; Destination
	dcx h
	mov e,m
	dcx h
	mov a,m
	dcx h
	mov l,m
	mov h,a
	;
	;	So after all that work we have HL=src DE=dst BC=count
	;	and we know count ;= 0.
	;
	;	Simple unoptimized copy loop for now. Horribly slow for
	;	things like 512 byte disk blocks
	;
uputcopy:
	mov a,m
	call map_proc_always
	stax d
	call map_kernel
	inx h
	inx d
	dcx b
	mov a,b
	ora c
	jnz uputcopy
	pop b
	lxi h,0
	ret

.export __uzero

__uzero:
	push b
	lxi h,4
	dad sp
	mov e,m
	inx h
	mov d,m
	inx h 
	mov c,m
	inx h
	mov b,m
	xchg
	
	mov a,b
	ora c
	jz nowork
;
;	Simple loop. Wants unrolling a bit
;
	call map_proc_always
	mvi d,0
zeroloop:
	mov m,d
	inx h
	dcx b
	mov a,b
	ora c
	jnz zeroloop
	pop b
	jmp map_kernel

