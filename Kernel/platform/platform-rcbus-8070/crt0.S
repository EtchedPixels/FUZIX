;
;	So we end up first in the image. Our loader is up top and we
;	already in all RAM mode
;

		.code

	.byte	0
start:	jmp entry
	jmp interrupt_handler
	jmp nmi_handler		; Not used/expected
entry:
	and	s,=0xFE		; interrupts off
	ld	p1,=kstack_top	; C stack
	ld	p2,=_udata
	ld	t,=0
	ld	ea,t
	st	a,:__tmp
wipeud:
	ld	ea,t
	st	ea,@2,p2
	ild	a,:__tmp
	bnz	wipeud

	ld	p2,=__bss
	ld	ea,=__bss_size
	st	a,:__tmp	; low part of loop sequence
	add	ea,=0xFF	; adjust upper
	xch	a,e
	st	a,:__tmp+1
bss_wipe:
	ld	ea,t
	st	a,@1,p2
bss_next:
	dld	a,:__tmp
	bnz	bss_wipe
	dld	a,:__tmp+1
	bnz	bss_wipe
gogogo:
	jsr	init_early
	jsr	init_hardware
	jsr	_fuzix_main	; Should never return
	and	s,=0xFE		; Spin
stop:
	bra	stop
