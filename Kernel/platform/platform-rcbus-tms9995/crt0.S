;
;	So we end up first in the image
;

		.code

		.byte 0x99
		.byte 0x95

entry:
	lwpi	0xF000
	limi	0
	li	r13, kstack_top	; C stack
	li	r0, _udata
	clr	r3			; R3 is a general zero for the crt
wipeud:
	mov	r3, *r0+
	ci	r0, _udata + 512
	jne	@wipeud

	li	r1,__bss_size
	li	r0,__bss
	inc	r1
	andi	r1, 0xFFFE
bss_wipe:
	mov	r3,*r0+
	dect	r1
	jne	@bss_wipe

gogogo:
	bl	@init_early
	bl	@init_hardware
	bl	@_fuzix_main		; Should never return
	limi	0
stop:	jmp	@stop
