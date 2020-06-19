;
;	Closely related to the 6803/6303 code but there are ABi differences
; 	and a lot less instructions we can use!
;
	.export _di
	.export _ei
	.export _irqrestore
	.export _swab
	.export _doexec
	.export _sys_cpu
	.export _sys_stubs
	.export _sys_cpu_feat
	.export _set_cpu_type
	.export unix_syscall_entry
	.export interrupt_handler
	.export nmi_handler
	.export trap_handler

	.export outnewline
	.export outcharhex
	.export outstring
	.export outx
	.export outd

	.export preemption

#include "platform/kernel.def"
#include "kernel-6303.def"

	.code

_di:
	tpa		; return cc codes in D
	sei
	rts

_ei:
	cli
	rts

_irqrestore:
	tsx
	ldaa	2,x 
			; D holds the return from di where A is the cc
	tap		; we trash overflow and carry but they are assumed
	jmp ret1

_swab:
	tsx
	ldab	3,x
	ldaa	4,x
	jmp ret2

	.common

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
	blt outh2
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
	stx @tmp
	ldab @tmp
	bsr outcharhex
	ldab @tmp+1
	bsr outcharhex
outsdone:
	rts

outd:
	pshb
	psha
	tab
	bsr outcharhex
	pula
	psha
	bsr outcharhex
	pula
	pulb
	rts

deliver_signals:
	ldaa _udata+U_DATA__U_CURSIG
	bne deliver_signals_2
signal_raced:
	rts
deliver_signals_2:
	lsla
	tab
	ldaa #<_udata+U_DATA__U_SIGVEC
	aba
	ldaa #>_udata+U_DATA__U_SIGVEC
	adca #0
	stab @tmp+1
	staa @tmp
	ldx @tmp
	ldx 0,x
	beq signal_raced
	clrb
	pshb
	psha
	ldab #<signal_return
	ldaa #>signal_return
	psha
	pshb
	jmp ,x
signal_return:
	sei
	sts _udata+U_DATA__U_SYSCALL_SP
	lds #kstack_top
	jsr map_kernel_di
	jsr _chksigs
	jsr map_process_always_di
	lds _udata+U_DATA__U_SYSCALL_SP
	bra deliver_signals
	

;
;	We are called from SWI. The stack holds 7 bytes of return
;	information, A holds the syscall number on entry, B the arg count
;	Arguments are left to right ordered.
;
;	On enry our frame looks like this
;
;	12->	more arguments
;	11-10	last arg
;	9-8	return PC for caller to syscall
;	7-6	UserPC
;	5-4	X
;	3	A
;	2	B
;	1	CC
;
;	We do TSX, ABX so that our arguments are the same X offset
;
unix_syscall_entry:
	sts @tmp		; No ABX so we have to do a bit of work
	ldaa @tmp+1
	aba
	staa @tmp+1
	bcc noinc
	inc @tmp
noinc:
	ldx @tmp
	stx _udata+U_DATA__U_SYSCALL_SP
	staa _udata+U_DATA__U_CALLNO
	ldaa 8,x
	ldab 9,x
	staa _udata+U_DATA__U_ARGN
	stab _udata+U_DATA__U_ARGN+1
	ldaa 6,x
	ldab 7,x
	staa _udata+U_DATA__U_ARGN+2
	stab _udata+U_DATA__U_ARGN+3
	ldaa 4,x
	ldab 5,x
	staa _udata+U_DATA__U_ARGN+4
	stab _udata+U_DATA__U_ARGN+5
	ldaa 2,x
	ldab 3,x
	staa _udata+U_DATA__U_ARGN+6
	stab _udata+U_DATA__U_ARGN+7
	ldaa #1
	staa _udata+U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	lds #kstack_top
	jsr map_kernel_di		; no-op in pure banked
	cli
	jsr _unix_syscall
	sei
	clr _udata+U_DATA__U_INSYS
	jsr map_process_always	; no-op in pure banked
	lds _udata+U_DATA__U_SYSCALL_SP
	; Now the fun bit - the CPU saves X and D so we have to patch them
	; into the return frame
	ldaa _udata+U_DATA__U_RETVAL
	ldab _udata+U_DATA__U_RETVAL+1
	tsx
	stab 2,x			; return in D
	staa 3,x
	ldaa _udata+U_DATA__U_ERROR
	ldab _udata+U_DATA__U_ERROR+1
	staa 4,x				; error in X
	stab 5,x
	tst _udata+U_DATA__U_CURSIG
	bne via_signal
	rti
via_signal:
	jsr deliver_signals_2
	rti

_doexec:
	tsx
	ldx 3,x
	sei
	jsr map_process_always
	lds _udata+U_DATA__U_ISP
	des				; offset by 1 on the 680X
	clr _udata+U_DATA__U_INSYS
	cli
	ldaa #>PROGLOAD
	ldab #<PROGLOAD
	jmp 0,x

trap_handler:
	tst _udata + U_DATA__U_INSYS
	bne trap_illegal
	tst _inint
	bne trap_illegal
	clra
	ldab #9
	pshb
	psha
	ldx _udata+U_DATA__U_PTAB
	ldab P_TAB__P_PID_OFFSET,x
	ldaa P_TAB__P_PID_OFFSET+1,x
	pshb
	psha
	ldaa #39
	ldab #04	; kill with 2 args
	swi		; Will never return

trap_illegal:
	ldx #illegalmsg
trapx:
	jsr outstring
	jsr _platform_monitor

nmi_handler:
	jsr map_kernel_di
	ldx #nmimsg
	bra trapx

illegalmsg:
	.ascii '[illegal]'
	.byte 0
nmimsg:
	.ascii '[NMI]'
	.byte 0
;
;	The trap saved our entire register state for us
;
interrupt_handler:
	sts istack_switched_sp
	lds #istack_top
	jsr map_save_kernel

	ldaa #1
	staa _udata+U_DATA__U_ININTERRUPT
	staa _inint
	; Save the C direct page values so we can re-enter
	; If we turn on register variables we will need to save them too
	ldaa @tmp
	psha
	ldaa @tmp+1
	psha
	ldaa @tmp1
	psha
	ldaa @tmp1+1
	psha
	ldaa @tmp2
	psha
	ldaa @tmp2+1
	psha
	ldaa @sreg
	psha
	ldaa @sreg+1
	psha
	ldaa @fp
	psha
	ldaa @fp+1
	psha
	jsr _platform_interrupt
	; Restore the C direct page
	pula
	staa @fp+1
	pula
	staa @fp
	pula
	staa @sreg+1
	pula
	staa @sreg
	pula
	staa @tmp2+1
	pula
	staa @tmp2
	pula
	staa @tmp1+1
	pula
	staa @tmp1
	pula
	staa @tmp+1
	pula
	staa @tmp
	tst _udata+U_DATA__U_INSYS
	bne iretk
	clr _inint
	tst _need_resched
	bne preemption
	jsr map_process_always
	lds istack_switched_sp
	clr _udata+U_DATA__U_ININTERRUPT
	tst _udata+U_DATA__U_INSYS
	bne no_signals
	jsr deliver_signals
no_signals:
	rti
	;
	; Return to kernel mode
	; We don't check signals in this case and we use map_restore
	;
iretk:
	jsr map_restore
	lds istack_switched_sp
	clr _udata+U_DATA__U_ININTERRUPT
	tst _udata+U_DATA__U_INSYS
	rti


preemption:
	clr _need_resched
	; Save the stack pointer across
	ldaa istack_switched_sp
	staa _udata+U_DATA__U_SYSCALL_SP
	ldaa istack_switched_sp+1
	staa _udata+U_DATA__U_SYSCALL_SP+1
	lds #kstack_top
	ldab #1
	stab _udata+U_DATA__U_INSYS
	jsr _chksigs
	ldx _udata+U_DATA__U_PTAB
	ldab P_TAB__P_STATUS_OFFSET,x
	cmpb #P_RUNNING
	bne not_running
	ldab #P_READY
	stab P_TAB__P_STATUS_OFFSET,x
	ldaa P_TAB__P_FLAGS_OFFSET,x
	orab #PFL_BATCH
	stab P_TAB__P_FLAGS_OFFSET,x
not_running:
	jsr _platform_switchout
	clr _udata+U_DATA__U_ININTERRUPT
	clr _udata+U_DATA__U_INSYS
	jsr map_process_always
	lds _udata+U_DATA__U_SYSCALL_SP
	ldaa _udata+U_DATA__U_CURSIG
	beq no_signals
	jsr deliver_signals_2
	rti

	.code

;
;	FIXME: eventually test 6803 v 6303 and for 6303
;	set features to 3 (xgdx as well as 6803) 
;

	.setcpu 6303

_set_cpu_type:
	rts
	; TODO 6800 v 6803
	clrb
	ldx #1
	xgdx		; appears to do nothing on the 6803
	tstb
	beq is_6803
	ldab #3		; 6303 feature set also present
	stab _sys_cpu_feat
is_6803:
	rts

	.setcpu 6800

	.data
_sys_cpu:
	.byte 2		; 6800 class CPU
_sys_cpu_feat:
	.byte 0		; 6800
_sys_stubs:
	swi
	rts
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
