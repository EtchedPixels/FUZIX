	.code

.export __text

__text:
	.word 0x80A8	; Fuzix executable
	.byte 5			; Z8 series
	.byte 0			; No feature set
	.byte 0			; Base 0x0000
	.byte 0			; No hints

	.word __data		; code + literal (0 based)
	.word __data_size	; data
	.word __bss_size	; bss size
	.byte 16		; Run 18 bytes in
	.byte 0			; No size hint
	.byte 0			; No stack hint
	.byte 0			; No ZP on Z8 (for now.. we may use it for regs

start:
	; Deal with relocatable binaries later
	ld r14,254
	ld r15,255
	; Top of stack is arguments then environ
	incw r14
	incw r14
	; Now points at argv
	lde r2,@rr14
	incw r14
	lde r3,@rr14
	ld r12,#>___argv
	ld r13,#<___argv
	lde @rr12,r2
	incw rr12
	lde @rr12,r3
	incw rr14		; point at environment block
	ld r12,#>_environ
	ld r13,#<_environ
	lde @rr12,r14
	incw rr12
	lde @rr15,r15
	call _main
	push r3
	push r2
	call _exit

	.data
					; kernel helpers
.export _environ
_environ:
	.word 0

