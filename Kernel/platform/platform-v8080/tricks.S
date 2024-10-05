#include "../../cpu-8080/kernel-8080.def"
#include "../../lib/8080fixedbank.S"

	.common

	.export bankfork

bankfork:
	sta patch1+1
	mov a,c
	sta patch2+1
	lxi h,0
	dad sp
	shld copy_done+1	; patch stack restore in

	; Go from the break to 0-5
	lhld U_DATA__U_BREAK
	lxi d,-6		; move down 6 for the copier loop
	dad d
	sphl
	mvi a,0xff		; end between 5 and 0 (which is fine)
	sta patch3+1
	lxi h,copy_stack
	jmp copier
	;
	;	Go from BE00 to the stack pointer
	;
copy_stack:
	lxi sp,0xBE00-6
	; Trickier .. need to work out where to stop
	lhld U_DATA__U_SYSCALL_SP
	lxi d,-0x0106		; 6 for the underrun 0x100 for the round down
	dad d
	mov a,h
	sta patch3+1
	lxi h,copy_done
	jmp copier
copy_done:
	lxi h,0
	sphl
	ret

copier:
	shld patch4+1
loop:
				; sp points to top of block
patch1:
	mvi a,0			;					7
	out 21			; source bank				10
	pop h			;					10
	pop d			;					10
	pop b			;					10
patch2:
	mvi a,0			;					7
	out 21			; dest bank				10
	push b			;					11
	push d			;					11
	push h			; sp now back where it started		11
	lxi h,-6		;					10
	dad sp			;					10
	sphl			; sp ready for next burst		5
	mov a,h			;					5
patch3:
	cpi 0			; wrapped to FFFx			7
	jnz loop		;					10

;
;	144 cycles per 6 bytes = 24 per byte which is actually not far off
;	a naive Z80 implementation and about half a good one. Still means
;	a second to do the fork() bank copy on a 1MHz 8080. Not quite so bad
;	on a 6MHz 8085 though 8)
;
;	We halt at somewhere around xx05-xx00 so we have to tidy up by hand
;	or accept an underrun. We go the overlap approach on the grounds
;	it's cheap and our main overcopy is at most 5 bytes in common,
;	whlist the bank to bank overcopy is harmless and small
;
;
	xra a
	out 21
patch4:
	jmp 0
