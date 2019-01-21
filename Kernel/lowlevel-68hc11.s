
	.file "lowlevel-68hc11"
	.mode mshort


	.globl di
	.globl ei
	.globl irqrestore

	.globl unix_syscall_entry
	.globl dispatch_process_signal
	.globl interrupt_handler

	.globl outnewline
	.globl outcharhex
	.globl outstring
	.globl outx
	.globl outy
	.globl outd

	include "platform/kernel.def"
	include "kernel-hc11.def"

di:
	tpa		; return cc codes in D
	sei
	rts

ei:
	cli
	rts

irqrestore:		; D holds the return from di where A is the cc
	tap		; we trash overflow and carry but they are assumed
	rts		; clobbered anyway





outnewline:
	ldab #0x0d
	bsr outchar_call
	ldab #0x0a
	bra outchar_call


outcharhex:
	pshb
	lsrb
	lsrb
	lsrb
	lsrb
	bsr outnibble
	pulb
	pshb
	bsr outnibble
	pulb
	rts

outnibble:
	andb #0x0F
	cmpb #0x0A
	ble outh2
	addb #0x07
outh2:	addb #0x30
outchar_call:
	jmp outchar

outstring:
	ldab ,x
	beq outsdone
	bsr outchar_call
	inx
	bra outstring

outx:
	xgdx
	pshx		; actually the old D
	bsr outcharhex
	tab
	bsr outcharhex
	pulx
	xgdx
outsdone:
	rts

outy:
	xgdy
	pshy		; actually the old D
	bsr outcharhex
	tab
	bsr outcharhex
	puly
	xgdy
	rts

outd:
	psha
	pshb
	bsr outcharhex
	tab
	bsr outcharhex
	pulb
	pula
	rts


;
;	This is slightly odder than most platforms. At the point we
;	are called the arguments should already be in U_foo as we may be
;	doing a pure bank switch environment and will switch to the kernel
;	stack before this far call via the EEPROM
;
unix_syscall_entry:
	ldaa #1
	staa U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	jsr map_kernel		; no-op in pure banked
	cli
	jsr unix_syscall
	sei
	clr U_DATA__U_INSYS
	jmp map_process_always	; no-op in pure banked
	; signal processing happens in per platform code in case we are
	; pure banked
	; Caller must save errno and return value before invoking signal
	; processing.

;
;	May be a trampoline via a far call and bank switch, but not our
;	problem in this code. We do however assume the caller stack switched
;	for us to the IRQ stack
;
interrupt_handler:
	ldaa #1
	staa U_DATA__U_ININTERRUPT
	jsr map_save
	jsr map_kernel
	ldaa #1
	staa inint
	jsr platform_interrupt
	clr inint
	tst need_resched
	beq noswitch
	clr need_resched
	; Save the stack pointer across
	ldd istack_switched_sp
	std U_DATA__U_SYSCALL_SP
	lds #kstack_top
	jsr chksigs
	ldx U_DATA__U_PTAB
	ldab P_TAB__P_STATUS_OFFSET,x
	cmpb #P_RUNNING
	bne not_running
	ldab #P_READY
	stab P_TAB__P_STATUS_OFFSET,x
not_running:
	jsr platform_switchout
	jsr map_process_always
	; caller will switch back to stack in X
	ldx U_DATA__U_SYSCALL_SP
	rts
noswitch:
	jsr map_restore
	ldx istack_switched_sp
	rts			; caller will do the final stack flip

nmi_handler:
	lds #istack_top - 2
	jsr map_kernel
	ldx #nmimsg
	jsr outstring
	jsr platform_monitor

nmimsg:
	.ascii "[NMI]"
	.byte 13,10,0

;
;	Runs in kernel banking
;
dispatch_process_signal:
	ldab U_DATA__U_CURSIG
	bne dosig
	rts
dosig:
	clr U_DATA__U_CURSIG
	clra
	ldx #U_DATA__U_SIGVEC
	abx
	abx
	ldd ,x
	jmp sigdispatch		; platform provides. Calls d on the
				; user bank and stack. If it returns
				; then will go back via
				; dispatch_process_signal


;
;	Illegal instruction trap helper. Should send a signal
;	
sigill:
	rts
	

;
;	Support routines (FIXME copy over)
;
	.globl ___ashrsi3
	.globl ___ashlsi3
	.globl ___lshlhi3
	.globl ___lshlsi3
	.globl ___lshrhi3
	.globl ___lshrsi3
	.globl ___one_cmplsi2
	.globl ___mulhi3


___ashrsi3:
___ashlsi3:
___lshlhi3:
___lshlsi3:
___lshrsi3:
___lshrhi3:
___one_cmplsi2:
___mulhi3:
	rts
