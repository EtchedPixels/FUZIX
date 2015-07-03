
.text
.globl _syscall_return
_syscall_return:
        ; On exit from the kernel, the result is in r12 and r13 is an errno.
		; The system call number is still on the stack.
		incd sp

        tst r13
        jz 1f
		; Error path.
        mov r13, &errno
        mov #-1, r12
1:
        ret

