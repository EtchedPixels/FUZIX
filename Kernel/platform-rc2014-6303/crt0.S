;
;	So we end up first in the image
;
		.byte 0x03
		.byte 0x63

entry:
		sei			; interrupts off
		lds #kstack_top		; C stack
		ldx #_udata
wipeud:
		clr 0,x
		inx
		cpx #istack_switched_sp
		bne wipeud

		lda #<__BSS_RUN__
		sta ptr1
		lda #>__BSS_RUN__
		sta ptr1+1

		ldx #__bss
		ldd #__bss_size

bss_wipe:	clr ,x
		inx
		subd #1
		bne bss_wipe

gogogo:
		jsr init_early
		jsr init_hardware
		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop

; FIXME: linker
;		.segment "VECTORS"
		.word	vector
		.word	entry		; does it matter ???
		.word	nmi_handler
