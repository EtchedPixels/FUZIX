;
;	The interrupt page. We fold IIR and EIR together
;
;	For interrupts we should never get we do the required
;	irq clearing reads.
;
		.module iir

		.area _DISCARD

		.globl _iir

		.globl unix_syscall_entry
		.globl ticktock
		.globl sera_rx,serc_rx,serd_rx
		.globl sera_tx,serc_tx,serd_tx

		.include 'kernel.def'
		.include '../kernel-rabbit.def'

_iir:

periodic:
eir0:				; If we use both we'll need to do some
				; thinking and maybe split this
	jp ticktock

	.bndry 16
eir1:
	ipres
	ret

	.bndry 16

rst10:
	ret

	.bndry 16

rst18:
	ret

	.bndry 16

rst20:
	ret

	.bndry 16

rst28:
	jp unix_syscall_entry

	.bndry 16

spare:
	ret
	.bndry 16

rst38:
	ret

	.bndry 16

slave:
	ipres
	ret

	.bndry 16

spare2:
	ret

	.bndry 16

timera:
	push af
	ioi
	ld a,(TACSR)
	pop af
	ipres
	ret

	.bndry 16

timerb:
	push af
	ioi
	ld a,(TBCSR)
	pop af
	ipres
	ret

	.bndry 16

seriala:
	push af
	ioi
	ld a,(SASR)
	or a,a
	jp m,sera_rx
	jp sera_tx

	.bndry 16

;
;	B is our SPI
;
serialb:
	ipres
	ret

	.bndry 16

serialc:
	push af
	ioi
	ld a,(SCSR)
	or a,a
	jp m,serc_rx
	jp serc_tx


	.bndry 16

seriald:
	push af
	ioi
	ld a,(SDSR)
	or a,a
	jp m,serd_rx
	jp serd_tx


	.bndry 16
