		.code

.export __text		; We will need this for the syscall changes
			; to get at the stubs
__text:
		.word 0x80A8	; Fuzix executable
		.byte 1			; 8080 series
		.byte 0			; 8080 feature set
		.byte 1			; Base 0x100
		.byte 0			; No hints

		.word __data-0x100		; code
		.word __data_size	; data
		.word __bss_size	; bss size

		.byte 18		; Run 18 bytes in
		.byte 0			; No size hint
		.byte 0			; No stack hint
		.byte 0			; No ZP on 8080

		.word __sighandler		; signal handler vector

start2:
		lxi h,4
		dad sp
		shld _environ
		pop d			; argc
		pop h			; argv
		push h
		shld ___argv		; needed for stuff like err()
		push d
		call _main		; go
		push h
		call _exit


;
;	Signal handling glue. Called to indirect signals from the kernel
;	through code that saves the non-rentrant OS glue. Our stack
;	on entry has the return frame and we are passed C=signal number
;
__sighandler:
		; DE holds the vector, the stack is as we want it
		; but we have temporaries so must save them first
		lhld	__tmp
		push	h
		lhld	__hireg
		push	h
		lhld	__tmp2
		push	h
		lhld	__tmp2+2
		push	h
		lhld	__tmp3
		push	h
		lhld	__tmp3+2
		push	h

		mvi	b,0
		push	b		; signal number
		xchg
		call	__callhl
		pop	b

		; Recover compiler temporaries

		pop	h
		shld	__tmp3+2
		pop	h
		shld	__tmp3
		pop	h
		shld	__tmp2+2
		pop	h
		shld	__tmp2
		pop	h
		shld	__hireg
		pop	h
		shld	__tmp
		ret			; to the previous frame via the
					; kernel helpers
.export _environ
_environ:	.word 0

