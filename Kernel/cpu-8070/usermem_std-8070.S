	.export __uputc
	.export __uputw
	.export __ugetc
	.export __ugetw
	.export __uzero
	.export __uget
	.export __uput

	.common

__uputc:
	ld	ea,4,p1
	ld	p3,ea
	ld	a,2,p1
	jsr	map_proc_always
	st	a,0,p3
	jmp	map_kernel

__uputw:
	ld	ea,4,p1
	ld	p3,ea
	ld	ea,2,p1
	jsr	map_proc_always
	st	ea,0,p3
	jmp	map_kernel

__ugetc:
	ld	ea,2,p1
	ld	p3,ea
	jsr	map_proc_always
	ld	ea,=0
	ld	a,0,p3
	jmp	map_kernel

__ugetw:
	ld	ea,2,p1
	ld	p3,ea
	jsr	map_proc_always
	ld	ea,0,p3
	jmp	map_kernel

__uzero:
	ld	ea,2,p1
	ld	p3,ea
	ld	ea,4,p1
	add	ea,=0x010	; inc high half
	add	a,=0x01		; inc low half
	st	ea,:__tmp
	
	jsr	map_proc_always

	ld	t,=0
	bra	zloop
	; Optimize, unroll etc- certainly do word at a time!
uzerolp:
	ld	ea,t
	st	a,@1,p3
zloop:
	dld	a,:__tmp
	bnz	uzerolp
	dld	a,:__tmp+1
	bnz	uzerolp
	ld	ea,=0
	jmp	map_kernel

;
;	Also could do with optimized forms but these are platform specific
;	if so anyway!
;
__uget:
	ld	ea,4,p1
	ld	p2,ea		; copy to p2
	ld	ea,6,p1
	ld	p3,ea		; from p3
	ld	ea,2,p1		; for ea bytes
	add	ea,=0x0100
	add	a,=0x01
	bra	ugnext

ugloop:
	jsr	map_proc_always
	ld	a,@1,p3
	jsr	map_kernel
	st	a,@1,p2
ugnext:
	dld	a,:__tmp
	bnz	ugloop
	dld	a,:__tmp+1
	bnz	ugloop

	ld	ea,=0
	ret

__uput:
	; This one is harder as we've got to juggle two pointers
	; We can optimize this a lot by abusing the stack etc but
	; start simple. Take care as tmp/to/from are kernel mapped
	ld	ea,4,p1
	ld	p2,ea		; copy to p2
	ld	ea,6,p1
	ld	p3,ea		; from p3
	ld	ea,2,p1		; for ea bytes
	add	ea,=0x0100
	add	a,=0x01
	bra	upnext

uploop:
	ld	a,@1,p3
	jsr	map_proc_always
	st	a,@1,p2
	jsr	map_kernel
upnext:
	dld	a,:__tmp
	bnz	uploop
	dld	a,:__tmp+1
	bnz	uploop

	ld	ea,=0
	ret

