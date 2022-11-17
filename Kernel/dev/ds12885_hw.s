        .module ds12885_hw

        ; exported symbols
        .globl _ds12885_read
        .globl _ds12885_write

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; DS12885 interface
; -----------------------------------------------------------------------------

; Enter:
;	port on stack
; Exit:
;	L = data
_ds12885_read:
	pop hl	; bank
	pop bc	; return addr
	pop de	; port -> e
	push de
	push bc
	push hl
	ld c, #RTC_ADDR
	out (c), e		; set register
	ld c, #RTC_DATA
	in l, (c)		; read data
	ret

; Enter:
;	port, val on stack
_ds12885_write:
	pop hl	; bank
	pop bc	; return addr
	pop de	; port -> e, value -> d
	push de
	push bc
	push hl
	ld c, #RTC_ADDR
	out (c), e		; set register
	ld c, #RTC_DATA
	out (c), d		; write data
	ret
