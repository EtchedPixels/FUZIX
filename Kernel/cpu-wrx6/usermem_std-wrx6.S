	.export __uputc
	.export __uputw
	.export __ugetc
	.export __ugetw
	.export __uzero
	.export __uget
	.export __uput

	.common

	.setcpu	6

__uputc:
	stx	(-s)
	ldb	6(s)
	ldx	4(s)
	jsr	map_proc_always
	stbb	(x)
	jsr	map_kernel
	ldx	(s+)
	rsr

__uputw:
	stx	(-s)
	ldb	6(s)
	ldx	4(s)
	jsr	map_proc_always
	stb	(x)
	jsr	map_kernel
	ldx	(s+)
	rsr

__ugetc:
	ldb	2(s)
	jsr	map_proc_always
	ldbb	(b)
	jsr	map_kernel
	rsr

__ugetw:
	ldb	2(s)
	jsr	map_proc_always
	ldb	(b)
	jsr	map_kernel
	rsr

__uzero:
	stx	(-s)
	ldb	6(s)
	ldx	4(s)
	bnz	noworkz
	jsr	map_proc_always
	clab
	; FIXME: do word alignment and word clears
uzerolp:
	stab	(x+)
	dcr	b
	bnz	uzerolp
	jsr	map_kernel
noworkz:
	ldx	(s+)
	rsr
;
;	Dumb common implementation. Can often be done better by platform
;	Should also do multi-reg blocks and alignment here
;
__uget:
ugetl:
	stx	(-s)
	xfr	y,a
	sta	(-s)

	ldx	6(s)		; user
	lda	8(s)		; dest
	xay
	lda	10(s)		; count

ugetlp:
	sta	(-s)
	jsr	map_proc_always
	ldbb	(x+)
	jsr	map_kernel
	stbb	(y+)
	lda	(s+)
	dca
	bnz	ugetlp
	lda	(s+)
	xay
	ldx	(s+)
	clr	b
	rsr

__uput:
uputl:
	stx	(-s)
	xfr	y,a
	sta	(-s)

	ldx	6(s)		; user
	lda	8(s)		; dest
	xay
	lda	10(s)		; count

uputlp:
	sta	(-s)
	ldbb	(x+)
	jsr	map_proc_always
	stbb	(y+)
	jsr	map_kernel
	lda	(s+)
	dca
	bnz	uputlp
	lda	(s+)
	xay
	ldx	(s+)
	clr	b
	rsr
