#
	#include "../kernel-8085.def"
	#include "kernel.h"

	.setcpu 8085

	.code

	.export _ds12885_read
	.export _ds12885_write

; -----------------------------------------------------------------------------
; DS12885 interface
; -----------------------------------------------------------------------------

; Enter:
;	port on stack
; Exit:
;	L = data
_ds12885_read:
	ldsi 2
	ldax d
	out RTC_ADDR
	in RTC_DATA
	mov e,a
	mvi d,0
	ret

; Enter:
;	port, val on stack
_ds12885_write:
	ldsi 2
	ldax d
	out RTC_ADDR
	ldsi 4
	ldax d
	out RTC_DATA	
	ret
