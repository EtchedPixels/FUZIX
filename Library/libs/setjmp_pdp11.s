
	.text
	.even
	.globl _setjmp
	.globl _longjmp


_setjmp:
	mov 2(sp),r0
	mov r2,(r0)+
	mov r3,(r0)+
	mov r4,(r0)+
	mov r5,(r0)+
	mov r6,(r0)+
	clr r0
	rts pc

_longjmp:
	mov 2(sp),r1
	mov 4(sp),r0
	beq r0_ok
	inc r0
r0_ok:	mov (r1)+,r2
	mov (r1)+,r3
	mov (r1)+,r4
	mov (r1)+,r5
	mov (r1)+,r6
	rts pc
