
.text
.globl _syscall
_syscall:
        ; On entry, the syscall number is in r11 and the four parameters in
        ; r12-r15 (luckily, these are all caller saved).
        call #_start-2
        ; On exit from the kernel, the result in r12 is either 0 or an errno.
        tst r12
        jz 1f
		; Error path.
        mov r12, &errno
        mov #-1, r12
1:
        ret

