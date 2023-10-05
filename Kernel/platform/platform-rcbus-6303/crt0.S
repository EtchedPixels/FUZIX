;
;	So we end up first in the image
;

		.setcpu 6303
		.code

		.byte 0x03
		.byte 0x63

entry:
		sei			; interrupts off
		clra
		clrb
		std @zero
		incb
		stab @one+1		; set up '@one' for the compiler
		lds #kstack_top		; C stack
		ldx #_udata
wipeud:
		clr 0,x
		inx
		cpx #istack_switched_sp
		bne wipeud

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

