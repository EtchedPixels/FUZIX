;
;	So we end up first in the image
;

		.code

entry:
	lda	kstack_top
	xas
	di

	ldx	_udata
	lda	256
	clr	b
wipeud:
	stb	(x+)
	dca
	bnz	wipeud


	lda	__bss_size
	ldx	__bss

	clrb	bl
bss_wipe:
	stbb	(x+)
	dca	
	bnz	bss_wipe

gogogo:
	jsr	init_early
	jsr	init_hardware
	jsr	_fuzix_main		; Should never return
	di
stop:	bra	stop

