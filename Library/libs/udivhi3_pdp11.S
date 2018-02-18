/*
 *	The PDP 11 have a signed divide (except early machines) but not an
 *	unsigned one. For some reason the PDP-11 gcc support lib forgets to
 *	include this helper so we use our own
 *
 *	There are faster divide algorithms based upon lookup tables but they
 *	take a lot more memory.
 *
 *	WARNING: untested at this point
 */
	.text
	.even

	.globl ___divmodhi3
	.globl ___umodhi3
	.globl ___udivhi3
/*
 *	Maybe should panic ?
 */
divide_zero:
	clr r0
	clr r1
	rts pc

divops:
	mov $1,r0
	clr r1
	tst r3
	beq divide_zero
align:
	cmp r2,r3
	bhis divide	/* if its bigger we are done */
	bmi divide	/* if the top bit is set we are done */
	tst r0
	beq divide	/* and if we ran out of bits it is done */
	asl r0
	asl r3
	br align
divide:
	tst r0		/* keep going until we've dealt with each bit we
			   set in the first pass */
	beq donediv
	cmp r3,r2
	blo wentover
	sub r3,r2	/* set the bit and account for that chunk */
	bis r0,r1
wentover:
	clc
	ror r0
	clc
	ror r3
	br divide
donediv:
	mov r2,r1	/* makes it easier to handle */
	rts pc

/*
 *	Basically a C wrapper around the helper that returns the answer
 *	in r0 and r1.
 *
 *	For the later machines I'm not clear if this or somehow mangling div
 * 	into udiv is faster.
 */
___divmodhi3:
	mov r2,-(sp)
	mov r3,-(sp)
	mov 8(sp),r2
	mov 10(sp),r3
	jsr pc,divops
	mov (sp)+,r3
	mov (sp)+,r2
	rts pc
/*
 *	And we get both forms just by looking at r0 or r1
 */
___umodhi3:
	jsr pc,___divmodhi3
	mov r1,r0
	rts pc
___udivhi3:
	jsr pc,___divmodhi3
	rts pc
