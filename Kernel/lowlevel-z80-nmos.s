		.area _COMMONMEM

_di:		xor a		; NMOS Z80 bug work around as per CPU manual
		push af
		pop af		; clear byte on stack below our usage
		ld a, i
		jp pe, was_ei	; P is now IFF2, if irqs on return is safe
		dec sp		; the CPU may have lied due to an erratum
		dec sp
		pop af		; see if anyone pushed a return address
		and a
		jr nz, was_ei	; someone did - IRQs were enabled then
		scf		; disabled
was_ei:		push af
		pop hl
		ret

_irqrestore:	pop hl
		pop af
		jr c, was_di
		ei
		jr irqres_out
was_di:		di
irqres_out:	push af
		jp (hl)

