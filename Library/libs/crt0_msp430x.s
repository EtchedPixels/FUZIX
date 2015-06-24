.section ".header"
.globl _start
_start:
	jmp 1f                             ; two bytes
	.byte 0                            ; one byte of padding
	.byte 'F', 'Z', 'X', '1'           ; magic starts at 0x8003

	.word 0 ; chmem
	.word __data_start
	.word __data_end
	.word __bss_end
	.word 0 ; spare

	.align 2
1:
	; Wipe BSS.
	movx.a #__bss_start, r12
	mov #0, 13
	movx.a #__bss_end, r14
	calla #memset

	; Pull environ off the stack.
	movx.a 0(sp), &environ

	; When main returns, jump to _exit.
	pushx.a #exit
	br.a #main
	
.globl environ
.comm environ, 2

