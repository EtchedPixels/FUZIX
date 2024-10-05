;
;	So we end up first in the image
;

		.section .text

entry:
		sei			; interrupts off
		lds #kstack_top		; C stack

		; Clear the udata (probably not needed as we did the move
		ldx #udata
wipeud:
		clr 0,x
		inx
		cpx #istack_switched_sp
		bne wipeud

		ldx #__bss_start
		ldd #__bss_size
bss_wipe:	clr ,x
		inx
		subd #1
		bne bss_wipe

gogogo:
		jsr init_early
		jsr init_hardware
		jsr fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop


;
;	Force our custom sections to be allocated so they appear in the
;	final binary. That was fun to find....
;
		.section .discard,"a"
		.section .common,"a"
		.section .commondata,"a"
