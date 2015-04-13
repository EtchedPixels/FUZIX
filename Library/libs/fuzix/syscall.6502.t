#include "numbers.h"

.export _@NAME@
.import __syscall
.import pushax
.proc _@NAME@
	#if __syscall_arg_@NAME@ == -1
		/* Vararg syscall. We are not entered as fastcall, in addition y holds
		 * the argument
		 * count for us already. All the work is already done */
	#else
		/* This is basically "fastcall to cdecl and set y" */
		#if __syscall_arg_@NAME@ > 0
			jsr pushax
		#endif
		ldy #(__syscall_arg_@NAME@ * 2)
	#endif
	ldx #__syscall_@NAME@
	jmp __syscall
.endproc

