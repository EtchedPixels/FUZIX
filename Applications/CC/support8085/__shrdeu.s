		.export __shrde
		.export __shrdeu
		.export __shru
		.setcpu 8085
		.code

__shru:
		xchg		; shift value into d
		pop	h	; return address into h
		xthl		; return up stack, h is now the value to shift

; We have a 16bit right signed shift only
__shrdeu:
		mov	a,h
		ora	a
		jm	shrneg
__shrde:
		mov	a,e
		ani	15
		rz		; no work to do
shrpl:
		arhl
shnext:
		dcr	a
shftr:		jnz	shrpl
		ret


; Negative number - do first shift, clear the top bit then fall into the
; main loop
shrneg:
		mov	a,e
		ani	15
		rz		; no work to do
		arhl
		mov	d,a	; save count
		mov	a,h
		ani	0x7F
		mov	h,a
		mov	a,d	; restore count
		jmp	shnext
