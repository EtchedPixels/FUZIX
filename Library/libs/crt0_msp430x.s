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
	mov #__bss_start, r12
	mov #__bss_end, r13
2:
	clr.b @r12
	inc r12
	cmp r12, r13
	jnz 2b

	; Pull environ off the stack.
	mov 0(sp), &environ

	; When main returns, jump to _exit.
	push #exit
	br #main
	
.globl environ
.comm environ, 2

