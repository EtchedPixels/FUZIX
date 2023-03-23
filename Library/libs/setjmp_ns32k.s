#
#	Not so pretty as the 68K because we have system
#	registers that have to be pulled into gp registers
#	but otherwise basically the same idea
#
#	Some other implementations point sp into the buffer
#	and save[]. While it's cute it doesn't work for us
#	because you can take signals in a Unix-like OS.
#
#	Preserve r2-r7, FP, SP.
#


	.globl _setjmp

_setjmp:
	movd	4(sp),r0	#	Buffer pointer
	movd	0(sp),0(r0)	#	Return address
	movd	r2,4(r0)	#	Save r2
	movd	r3,8(r0)	#	Save r3
	movd	r4,12(r0)	#	Save r4
	movd	r5,16(r0)	#	Save r5
	movd	r6,20(r0)	#	Save r6
	movd	r7,24(r0)	#	Save r7
	sprd	sp,r1
	movd	r1,28(r0)	#	SP
	sprd	fp,r1
	movd	r1,32(r0)	#	FP
	# We don't support FPU state here
	movqd	0,r0
	ret	0

	.globl _longjmp

_longjmp:
	movd	8(sp),r2
	cmpqd	0,r2
	bne	ok
	movqd	1,r2
ok:
	movd	4(sp),r0	#	Buffer
	movd	32(r0),r1
	lprd	fp,r1		#	Restore FP
	movd	28(r0),r1
	lprd	sp,r1		#	Restore SP
	movd	24(r0),r7	#	Registers
	movd	20(r0),r6
	movd	16(r0),r5
	movd	12(r0),r4
	movd	8(r0),r3
	# Now we have to juggle a bit as r2 currently holds our return value
	movd	0(r0),tos	#	Jump address
	movd	4(r0),tos
	movd	r2,r0
	movd	tos,r2
	ret	0
