.code 16
.text
.thumb_func
.align 4
.global _syscall_ret
# On entry:
#   r0 = return value
#   r1 = errno
_syscall_ret:
	cmp r1, #0
	beq _no_error
	ldr r2, =errno
	str r1, [r2]
_no_error:
	bx lr

