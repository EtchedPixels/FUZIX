	.export __uputc
	.export __uputw
	.export __ugetc
	.export __ugetw
	.export __uzero
	.export __uget
	.export __uput

	.common

;
;	Value r4 pointer r5
;
__uputc:
	mov	r11,r2
	bl	@map_proc_always
	swpb	r4		; it's word argument byte write
	movb	r4,*r5
	bl	@map_kernel
	b	*r2

__uputw:
	mov	r11,r2
	bl	@map_proc_always
	mov	r4,*r5
	bl	@map_kernel
	b	*r2

; Address in r4
__ugetc:
	mov	r11,r2
	bl	@map_proc_always
	clr	r1
	movb	*r4,r1
	bl	@map_kernel
	swpb	r1		; return is int
	b	*r2

__ugetw:
	mov	r11,r2
	bl	@map_proc_always
	mov	*r4,r1
	bl	@map_kernel
	b	*r2

;
;	r4 = dest, r5 = count
;
__uzero:
	mov	r11,r2
	bl	@map_proc_always
	clr	r1
	; FIXME: do word alignment and word clears
uzerolp:
	movb	r1,*r4+
	dec	r5
	jne	@uzerolp
	bl	@map_kernel
	b	*r2
;
;	Dumb common implementation. Can often be done better by platform
;	Should also do multi-reg blocks and alignment here
;
;	r4 = user r5 = dest *r13 = size
;
;
__uget:
ugetl:
	mov	r11,r2
	mov	*r13,r3
ugetlp:
	bl	@map_proc_always
	movb	*r4+,r1
	bl	@map_kernel
	movb	r1,*r5+
	dec	r3
	jne	@ugetlp
	clr	r1
	b	*r2
;
;	r4 = kernel r5=user *r13 = size
;	
__uput:
	mov	r11,r2
	mov	*r13,r3
uputlp:
	movb	*r4+,r1
	bl	@map_proc_always
	movb	r1,*r5+
	bl	@map_kernel
	dec	r3
	jne	@uputlp
	clr	r1
	b	*r2
