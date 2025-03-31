	.code

	.export __text		; We will need this for the syscall changes
				; to get at the stubs
__text:
	.word 0x80A8		; Fuzix executable
	.byte 10		; 8070 series
	.byte 0			; 8070 feature set
	.byte 1			; Base 0x100
	.byte 0			; No hints

	.word __data-0x100	; code
	.word __data_size	; data
	.word __bss_size	; bss size

	.byte 18		; Run 18 bytes in
	.byte 0			; No size hint
	.byte 0			; No stack hint

	.byte __zp_size		; Fixed size DP for compiler
	.word __sighandler	; signal handler vector

start2:
	jsr	___stdio_init_vars
	ld	ea,p1
	add	ea,=4
	ld	p3,=_environ
	st	ea,0,p3
	ld	ea,2,sp
	ld	p3,=___argv
	st	ea,0,p3
	jsr	_main		; go
	push	ea
	jsr	_exit


;
;	Signal handling glue. Called to indirect signals from the kernel
;	through code that saves the non-rentrant OS glue. Our stack
;	on entry has the return pointer and we are passed the signal number
;	on the stack and the vector in t
;
__sighandler:
	ld	ea,:__tmp
	push	ea
	ld	ea,:__tmp2
	push	ea
	ld	ea,:__hireg
	push	ea

	ld	ea,8,p1		; signal number
	push	ea

	ld	ea,=sigret
	push	ea		; return ptr

	ld	ea,t
	push	ea		; vector to call

	ret			; call the handler
sigret:
	pop	ea		; drop signal argument

	; Recover compiler temporaries
	pop	ea
	st	ea,:__hireg
	pop	ea
	st	ea,:__tmp2
	pop	ea
	st	ea,:__tmp
	ret			; to the previous frame via the
				; kernel helpers
	.export _environ

_environ:
	.word 0

	.dp

	.export	__tmp
	.export	__tmp2
	.export __hireg

__tmp:
	.word	0
__tmp2:
	.word	0
__hireg:
	.word	0
