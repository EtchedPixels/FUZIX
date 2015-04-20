		.area _COMMONMEM

		; IRQ helpers, in common as they may get used by common C
		; code (and are tiny)

_di:		ld a, i
		push af
		pop hl
		di
		ret

_irqrestore:	pop hl		; sdcc needs to get register arg passing
		pop de
		pop af		; so badly
		jp po, was_di
		ei
		jr irqres_out
was_di:		di
irqres_out:	push af
		push de
		jp (hl)

