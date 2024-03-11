#include "kernel-z8.def"

;
;	Simple implementation for now. Should be optimized
;

	.common

.export __uputc

__uputc:
	ld 	r15,#2
	call	__gargr4	; r0-r1 val, r2-r3 ptr
	call map_proc_always
	lde	@rr2,r1
	clr	r2
	clr	r3
	jp	map_kernel

.export __uputw

__uputw:
	ld 	r15,#2
	call	__gargr4	; r0-r1 val, r2-r3 ptr
	call map_proc_always
	lde	@rr2,r0
	incw	rr2
	lde	@rr2,r1
	clr	r2
	clr	r3
	jp	map_kernel

.export __ugetc

__ugetc:
	ld	r15,#2
	call	__gargr2
	call	map_proc_always
	lde	r3,@rr2
	clr	r2
	jp	map_kernel

.export __ugetw

__ugetw:
	ld	r15,#2
	call	__gargr2
	ld	r14,r2
	ld	r15,r3
	call	map_proc_always
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	jp	map_kernel

.export __uget

;
;	Stacked arguments are src.w, dst.w, count.w
;
__uget:
	ld	r12,254
	ld	r13,255
	incw	r12
	incw	r12
	lde	r14,@rr12	; source
	incw	r12
	lde	r15,@rr12
	incw	r12
	lde	r2,@rr12	; dest
	incw	r12
	lde	r3,@rr12
	incw	r12
	lde	r0,@rr12	; len
	incw	r12
	lde	r1,@rr2

	ld	r12,r0
	or	r12,r1
	jr	z, nowork
	;
	;	So after all that work we have HL=src DE=dst BC=count
	;	and we know count ;= 0.
	;
	;	Simple unoptimized copy loop for now. Horribly slow for
	;	things like 512 byte disk blocks
	;

	;	copy from r14/r15 to r2/r3 for r0/1
ugetcopy:
	call	map_proc_always
	lde	r12,@rr14
	call	map_kernel
	lde	@rr2,r12
	incw	rr14
	incw	rr2
	decw	rr0
	jr	nz,ugetcopy
	clr	r2
	clr	r3
	ret

.export __uput

__uput:
	ld	r12,254
	ld	r13,255
	incw	r12
	incw	r12
	lde	r14,@rr12	; source
	incw	r12
	lde	r15,@rr12
	incw	r12
	lde	r2,@rr12	; dest
	incw	r12
	lde	r3,@rr12
	incw	r12
	lde	r0,@rr12	; len
	incw	r12
	lde	r1,@rr2

	ld	r12,r0
	or	r12,r1
	jr	z, nowork
	;
	;	So after all that work we have HL=src DE=dst BC=count
	;	and we know count ;= 0.
	;
	;	Simple unoptimized copy loop for now. Horribly slow for
	;	things like 512 byte disk blocks
	;

	;	copy from r14/r15 to r2/r3 for r0/1
uputcopy:
	lde	r12,@rr14
	call	map_proc_always
	lde	@rr2,r12
	call	map_kernel
	incw	rr14
	incw	rr2
	decw	rr0
	jr	nz,uputcopy
nowork:
	clr	r2
	clr	r3
	ret

.export __uzero

__uzero:
	ld	r3,#2
	call	__gargr4	;	0-1 ptr 2-3 length
	
	ld	r12,r2
	or	r12,r3
	jr	z, nowork
;
;	Simple loop. Wants unrolling a bit and to use djnz
;
	call	map_proc_always
	clr	r12
zeroloop:
	lde	@rr0,r12
	incw	rr0
	decw	rr2
	jr	nz, zeroloop
	; r2/r3 is already zer
	jp	map_kernel
