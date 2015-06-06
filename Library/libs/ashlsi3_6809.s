	.globl	___ashlsi3

	.area .text

___ashlsi3:
	pshs	u

	; FIXME temporary hack until we fix gcc-6809 or our use of it
	; the argument passing doesn't match so we'll mangle it
	ldu 4,s
	stu ,x
	ldu 6,s
	stu 2,x
	ldb 9,s

	cmpb	#16
	blt	try8
	subb	#16
	; Shift by 16
	ldu	2,x
	stu	,x
	ldu	#0
	stu	2,x
try8:
	cmpb	#8
	blt	try_rest
	subb	#8
	; Shift by 8
	ldu	1,x
	stu	,x
	lda	3,x
	sta	2,x
	clr	3,x

try_rest:
	tstb
	beq	done
do_rest:
	; Shift by 1
	asl	3,x
	rol	2,x
	rol	1,x
	rol	,x
	decb
	bne	do_rest
done:
	puls	u,pc
