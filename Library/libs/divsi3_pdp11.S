		.text
		.even

		.globl ___divsi3
		.globl ___modsi3


___divsi3:
		mov r2,-(sp)
		mov 4(sp),r1
		mov 6(sp),r2
		clr r0
		div r0,r2
		mov (sp)+,r2
		rts pc

___modsi3:
		mov r2,-(sp)
		mov 4(sp),r1
		mov 6(sp),r2
		clr r0
		div r0,r2
		mov (sp)+,r2
		mov r1,r0
		rts pc
