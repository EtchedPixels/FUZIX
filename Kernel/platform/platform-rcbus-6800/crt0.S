;
;	So we end up first in the image
;

		.code

		.byte 0x00
		.byte 0x68

entry:
		sei			; interrupts off
		clra
		clrb
		stab @zero
		stab @zero+1
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
bss_wipe:	cpx #__end
		beq wiped
		clr ,x
		inx
		bra bss_wipe

wiped:
		jsr init_early
		jsr init_hardware
		jsr _fuzix_main		; Should never return
		sei			; Spin
stop:		jmp stop

