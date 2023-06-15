#include "../kernel-8080.def"

;
;	8080 port of the VDP driver code
;
;	The TMS9918A looks like a simple  I/O device but internally it's
;	actually attached to the CPU by various buffers and runs at a lower
;	pace than the CPU. A write is effectively queued with a depth of
;	one. Reads are more complicated. The I/O ports set an address which
;	is read at some point later and put into a buffer. We thus have to
;	not only wait but wait in the right part of the cycle.
;
;	The read/write buffer is shared
;
;	Vertical blank we can do much better but right now we don't do
;	anything like scroll or write on vblank. We should do to sort the
;	scroll flickering.
;
;	Timing rules
;	Text RAM access worst case ~26 clocks @ 7.4MHz
;
		.setcpu 8080

		.code
;
;	Register write value E to register A. This is a pure VDP register
;	access so we shouldn't need a delay (check if we need a small one
;	just because we are hitting it at nearly 8MHz)
;
;	We use a fixed port
;
vdpout:	    
		mov a, e
		out 0x99		; Data
		mov a, d
		out 0x99		; Register | 0x80
		ret

;
;	FIXME: need to IRQ protect the pairs of writes
;


videopos:	; turn E=Y D=X into HL = addr
	        ; pass B = 0x40 if writing
	        ; preserves C
		lda _inputtty
		dcr a
		add a			; 1K per screen
		add a
		add b
		mov b, a
		mov a, e			; 0-24 Y
		add a
		add a
		add a			; x 8
		mov l, a
		mvi h, 0
		push h
		dad h			; x 16
		dad h			; x 32
		mov a, d
		pop d
		dad d			; x 40
		mov e, a
		mov d, b		; 0 for read 0x40 for write
		dad d			; + X
		ret
;
;	Eww.. wonder if VT should provide a hint that its the 'next char'
;
		.export _plot_char

_plot_char:
		lxi	h,4
		dad	sp
		mov	a,m
		dcx	h
		mov	d,m
		dcx	h
		mov	e,m		; D = x E = y, A = char
		mov c, a

		lda _int_disabled
		push psw
		di
plotit:
		mvi b, 0x40		; writing
		call videopos		; preserves C
		mov a, l		; address low
		out 0x99
		mov a, h		; address high | 0x40
		out 0x99
		mov a, c
		out 0x98		; character to memory
		; We need 24 clocks for this to complete - which is fine as
		; we won't be back that fast
popret:
		pop psw
		ora a
		rnz
		ei
		ret

		.export _scroll_down

_scroll_down:
		lda _int_disabled
		push psw
		di
		mvi b, 23
		lxi d, 0x3C0		; start of bottom line
upline:
		push b
		mvi b, 40
		lxi h,scrollbuf
		mov a, e
		out 0x99		; our position
		mov a, d
		out 0x99
		; Wait for the TMS9918A to be ready - FIXME we can trim this
		; a tiny bit because there are a few machine cycles from the
	        ; out data hitting the TMS9918A to the in decoding and reading
		push b
		pop b			; kill 24 clocks
down_0:
		; Our data is not available for 24 clocks from the setup or
		; previous in. Not so big a problem on an 8080
		in 0x98
		mov m, a		; 7
		inx h			; 6
		dcr b			; 4
		jnz down_0		; 10 (27 clocks plus in so fine)
		lxi h, 0x4028		; go down one line and into write mode
		dad d			; relative to our position
		mov a, l
		out 0x99
		mov a, h
		out 0x99
		mvi b, 0x28		; 7
		lxi h, scrollbuf	; 10
down_1:
		mov a,m			; 7 which with the out decode is ok
		out 0x98		; video ptr is to the line below so keep going
		inx h			; 6
		dcr b			; 4
		jnz down_1		; 10

		pop b			; recover line counter
		lxi h, 0xffd8
		dad d			; up 40 bytes
		xchg			; and back into DE
		dcr b
		jnz upline
		jmp popret

		.export _scroll_up

_scroll_up:
		lda _int_disabled
		push psw
		di
		mvi b, 23
		lxi d, 40		; start of second line
downline:   	push b
		lxi h, scrollbuf
		mov a,e
		out 0x99
		mov a,d
		out 0x99
		push b			; kill 24 clocks
		pop b
up_0:
		in 0x98
		mov m,a			; 7
		inx h			; 6
		dcr b			; 4
		jnz up_0		; 10

		lxi h,0x3FD8		; up 40 bytes in the low 12 bits, add 0x40
					; for write ( we will carry one into the top
					; nybble)
		dad d
		mov a,l
		out 0x99
		mov a,h
		out 0x99
		lxi h, scrollbuf	; 10
		mvi b, 40		; 7
up_1:
		mov a,m			; 7
		out 0x98		; just about ok
		inx h			; 6
		dcr b			; 4
		jnz up_1		; 10
		pop b
		lxi h,40
		dad d
		xchg
		dcr b
		jnz downline
		jmp popret

		.export _clear_lines

_clear_lines:
		lxi	h,2
		dad	sp
		mov	e,m
		inx	h
		inx	h
		mov	d,m
	        xra a
	        cmp d
                rz
		lda _int_disabled
		push psw
		di
		mov c, d
		mvi d, 0
		mvi b, 0x40
		call videopos
		mov e, c
		mov a, l
		out 0x99
		mov a, h
		out 0x99
		nop			; 4
		nop			; 4
l0:
		mvi a, ' '		; 7
l2:		mvi b, 40  		; 7
l1:	    	out 0x98		; 10
					; Inner loop clears a line, outer counts
					; need 26 clocks between writes.
		dcr b			; 4
		nop			; 4
		jnz l1			; 10
		dcr e			; 4
		jnz l2			; 10
		jmp popret

		.export _clear_across

_clear_across:
		lxi	h,6		; low byte of arg 3 (len)
		dad	sp
		mov	a,m
		dcx	h
		dcx	h
		mov	e,m		; low byte of arg 2 (co-ords)
		dcx	h
		dcx	h
		mov	d,m		; and other co-ord
	        ora	a
		rz
		mov	c,a		; C = count

		lda _int_disabled
		push psw
		di
		mvi b, 0x40
		call videopos
		mov a, l
		out 0x99
		mov a, h
		out 0x99
		mov b, a		; 4
		mvi a, ' '		; 7
		nop			; 4
		nop			; 4
l3:		out 0x98		; 10
		dcr b			; 4
		nop			; 4
		jnz l3			; 10
		jmp popret

;
;	Turn on the cursor if this is the displayed console
;
		.export _cursor_on

_cursor_on:
		lxi	h,4
		dad	sp
		mov	e,m
		dcx	h
		dcx	h
		mov	d,m
		xchg			; HL is the cursor position

		lda _outputtty
		mov c, a
		lda _inputtty
		cmp c
		rnz

		lda _int_disabled
		di

		shld cursorpos

		mvi b,0x00		; reading
		xchg			; DE is now the cursor position
		call videopos

		mov a, l
		out 0x99		; address low
		mov a, h
		out 0x99		; address high
		push b			; Now kill 24 clocks
		pop b
		in 0x98			; character
		sta cursorpeek		; save it away

		mov c, a
		mov a, l
		out 0x99
		mov a, h
		adi 0x40		; make it a write command
		out 0x99
		mov a,c			; 4
		; FIXME delay
		xri 0x80		; 7 invert the video
		inx h			; Kill 12 clocks
		dcx h
		out 0x98		; character out
		jmp popret

		.export _cursor_off

_cursor_off:
		lda _outputtty
		mov c,a
		lda _inputtty
		cmp c
		rnz
		lda _int_disabled
		push psw
		lhld cursorpos
		xchg
		lda cursorpeek
		mov c,a
		jmp plotit

		.export _vtattr_notify
		.export _cursor_disable

_vtattr_notify:
_cursor_disable:
		ret

		.export _set_console

_set_console:
		lda _inputty
		dcr a
		out 0x99
		mvi a,0x82		; Register 2 (base)
		out 0x99
		ret
;
;	This must be in data or common not code
;
	    	.bss

cursorpos:  .word 0
cursorpeek: .byte 0
scrollbuf:  .ds 40
