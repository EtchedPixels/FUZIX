
.text
.globl _syscall
_syscall:
        ; On entry, the syscall number is in r11 and the four parameters in
        ; r12-r15 (luckily, these are all caller saved).
        call &(_start-2)
        ; On exit from the kernel, the result is in r12 and r13 is an errno.
        tst r13
        jz 1f
		; Error path.
        mov r13, &errno
        mov #-1, r12
1:
        ret

