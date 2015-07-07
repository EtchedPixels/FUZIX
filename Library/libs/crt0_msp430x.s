.section ".header"
.globl _start
_start:
	jmp 1f                             ; two bytes
	.byte 0                            ; two bytes of padding
	.byte 'F', 'Z', 'X', '1'           ; magic starts at 0x8003

	.byte __user_page                  ; page to start at
	.word 0                            ; chmem (0 means 'all')
	.word __data_start
	.word __data_len
	.word __bss_len
	.word 0 ; spare

1:
	; Wipe BSS.
	mov #__bss_start, r12
	mov #__bss_end, r13
2:
	clr.b @r12
	inc r12
	cmp r12, r13
	jnz 2b

	; Initialise stdio.
	call #__stdio_init_vars

	; Pull argc and argv off the stack.
	pop r12
	pop r13

	; What's left on the stack is the environment.
	mov sp, r14
	mov.w r14, &environ

	; When main returns, jump to _exit.
	push #exit
	br #main
	
.globl environ
.comm environ, 2

