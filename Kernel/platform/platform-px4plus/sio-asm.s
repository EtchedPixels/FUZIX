;
;	Asm support sections of the SIO logic
;

		.module sioasm

		.globl _sio_set_irq
		.globl _sio_release_irq
		.globl _sio_count

		.globl interrupt_handler

		.area _CODE

_sio_set_irq:
	ld a, #0x04
	out (4), a			; only IRQ is FRC
	ld hl, #0
	ld (_sio_count), hl
	ld hl, #interrupt_fast
	; A 16bit LD is not interruptible so this is safe */
	ld (0x0039), hl
	ret

;
;	Returns the number of missed interrupts in HL
;
_sio_release_irq:
	ld a, r
	push af
	di
	ld a, #0x0B			; FRC, ART and 7508
	out (4), a
	ld hl, #interrupt_handler
	ld (0x0039), hl
	ld hl, (_sio_count)
	pop af
	ret po		; CMOS
	ei
	ret

		.area _COMMONMEM
;
;	We need to do special IRQ handling when doing SIO transfers
;	At this point only the FRC overflow interrupt is enabled
;
interrupt_fast:
	    push af
	    push hl
	    ld a, #4
	    out (2), a			; ack the FRC interrupt
	    ld hl, #_sio_count
	    inc (hl)
	    pop hl
	    pop af
	    ret
_sio_count:
	    .db 0


