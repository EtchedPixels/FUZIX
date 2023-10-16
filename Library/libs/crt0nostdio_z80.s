		.code

.export __text		; We will need this for the syscall changes
			; to get at the stubs
__text:
		.word 0x80A8	; Fuzix executable
		.byte 1			; 8080 series
		.byte 0			; 8080 feature set
		.byte 1			; Base 0x100
		.byte 0			; No hints

		.word __data-0x100	; code
		.word __data_size	; data
		.word __bss_size	; bss size

		.byte 18		; Run 18 bytes in
		.byte 0			; No size hint
		.byte 0			; No stack hint
		.byte 0			; No ZP on 8080

		.word __sighandler		; signal handler vector

start2:
		ld hl,4
		add hl,sp
		ld (_environ),hl
		pop de			; argc
		pop hl			; argv
		push hl
		ld (___argv),hl		; needed for stuff like err()
		push de
		call _main		; go
		push hl
		call _exit


;
;	Signal handling glue. Called to indirect signals from the kernel
;	through code that saves the non-rentrant OS glue. Our stack
;	on entry has the return frame and we are passed C=signal number
;
__sighandler:
		; DE holds the vector, the stack is as we want it
		; but we have temporaries so must save them first
		ld	hl,(__tmp)
		push	hl
		ld	hl,(__hireg)
		push	hl
		ld	hl,(__tmp2)
		push	hl
		ld	hl,(__tmp2+2)
		push	hl
		ld	hl,(__tmp3)
		push	hl
		ld	hl,(__tmp3+2)
		push	hl

		ld	b,0
		push	bc		; signal number
		ex	de,hl
		call	__callhl
		pop	bc

		; Recover compiler temporaries

		pop	hl
		ld	hl,(__tmp3+2)
		pop	hl
		ld	hl,(__tmp3)
		pop	hl
		ld	hl,(__tmp2+2)
		pop	hl
		ld	hl,(__tmp2)
		pop	hl
		ld	hl,(__hireg)
		pop	hl
		ld	hl,(__tmp)
		ret			; to the previous frame via the
					; kernel helpers
.export _environ
_environ:	.word 0

