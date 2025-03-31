;
;	So we end up first in the image
;

		.code

	.byte	0x70
	.byte	0x80

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
	st	ea,@2,p2
	ild	a,:__tmp
	bnz	wipeud

	ld	p2,=__bss
	ld	ea,=__bss_size+0x0100
	add	a,=1
	st	ea,:__tmp
bss_wipe:
	ld	ea,t
	st	ea,@2,p2
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
