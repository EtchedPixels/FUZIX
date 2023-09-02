;
;	We use the same low level code. There is no particular reason to use
;	xgdx and friends here so 6803 and 6303 can share
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

	.setcpu 6803

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
	ldaa	3,x
			; D holds the return from di where A is the cc
	tap		; we trash overflow and carry but they are assumed
	rts		; clobbered anyway

_swab:
	tsx
	ldab	2,x
	ldaa	3,x
	rts

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
	pshb
	psha
	stx @tmp
	ldab @tmp
	bsr outcharhex
	ldab @tmp+1
	bsr outcharhex
	pula
	pulb
outsdone:
	rts

outd:
	psha
	pshb
	tab
	bsr outcharhex
	pulb
	pshb
	bsr outcharhex
	pulb
	pula
	rts

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
	tsx
	sts _udata+U_DATA__U_SYSCALL_SP
	abx
	staa _udata+U_DATA__U_CALLNO
	ldd 7,x
	std _udata+U_DATA__U_ARGN
	ldd 5,x
	std _udata+U_DATA__U_ARGN+2
	ldd 3,x
	std _udata+U_DATA__U_ARGN+4
	ldd 1,x
	std _udata+U_DATA__U_ARGN+6
	ldaa #1
	staa _udata+U_DATA__U_INSYS	; we may want to use udata-> tricks ?
	lds #kstack_top
	jsr map_kernel_di		; no-op in pure banked
	cli
	jsr _unix_syscall
	sei
	clr _udata+U_DATA__U_INSYS
	jsr map_proc_always	; no-op in pure banked
	lds _udata+U_DATA__U_SYSCALL_SP
	; Now the fun bit - the CPU saves X and D so we have to patch them
	; into the return frame
	ldd _udata+U_DATA__U_RETVAL
	tsx
	stab 1,x			; return in D
	staa 2,x
	ldd _udata+U_DATA__U_ERROR
	std 3,x				; error in X
rti_return:
	ldab _udata+U_DATA__U_CURSIG
	beq no_signal
	ldx #_udata+U_DATA__U_SIGVEC
	abx
	abx
	ldd 0,x
	beq no_signal
	clr 0,x
	clr 1,x
	std @tmp
	ldx @tmp
	ldab _udata+U_DATA__U_CURSIG
	clr _udata+U_DATA__U_CURSIG
	
	; Arguments to signal handler
	clra
	pshx
	; Unwind for signal handlers
	ldx PROGLOAD+16	; Helper vector FIXME: assumes loaded at base
	; We are off the kernel stack at this point so we can
	; enable interrupts and leap into userspace
	cli
	jsr ,x
	ins	; Drop vector
	ins
no_signal:
	rti

_doexec:
	tsx
	ldx 2,x
	sei
	jsr map_proc_always
	lds _udata+U_DATA__U_ISP
	des				; offset by 1 on the 680X
	clr _udata+U_DATA__U_INSYS
	cli
	pshx
	pula				; base is exec addr & FF00
	ins
	ldab #DPBASE			; DP base is set by the platform header
	jmp 0,x

trap_handler:
	tst _udata + U_DATA__U_INSYS
	bne trap_illegal
	tst _udata + U_DATA__U_ININTERRUPT
	bne trap_illegal
	; We should fix the trap address of the stack and see if it is > 31
	; if so then it was an illegal instruction on 6303 TODO
	ldx #9
	pshx
	ldx _udata+U_DATA__U_PTAB
	ldx P_TAB__P_PID_OFFSET,x
	pshx
	ldd #3904	; kill with 2 args
	swi		; Will never return

trap_illegal:
	ldx #illegalmsg
trapx:
	jsr outstring
	jsr _plt_monitor

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
	; Save the C direct page values so we can re-enter
	; If we turn on register variables we will need to save them too
	ldx @tmp
	pshx
	ldx @tmp1
	pshx
	ldx @tmp2
	pshx
	ldx @sreg
	pshx
	ldx @fp
	pshx
	jsr _plt_interrupt
	; Restore the C direct page
	pulx
	stx @fp
	pulx
	stx @sreg
	pulx
	stx @tmp2
	pulx
	stx @tmp1
	pulx
	stx @tmp
	; If we interrupted kernel mode then we just put the map back and
	; return
	tst _udata+U_DATA__U_INSYS
	bne iretk
	; If we had to reschedule it's more complicated
	tst _need_resched
	bne preemption
	; Put the process memory map back
	jsr map_proc_always
	; Put the process stack back
	lds istack_switched_sp
	; We are no longer on the istack or in an interrupt
	clr _udata+U_DATA__U_ININTERRUPT
	; Don't deliver signals if we were in a system call
	tst _udata+U_DATA__U_INSYS
	bne no_signals
	jmp rti_return
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
	rti


;
;	Task switch due to an interrupt.
;
preemption:
	; Acknowledge the request
	clr _need_resched
	; Copy the user stack save to the syscall save
	ldd istack_switched_sp
	std _udata+U_DATA__U_SYSCALL_SP
	; Move onto our process kernel stack
	; We can't stay on the istack as it'll be gone when
	; we return.
	lds #kstack_top
	; Make it look like we are in a syscall
	ldab #1
	stab _udata+U_DATA__U_INSYS
	; Update our signals
	jsr _chksigs
	; Correct our process state
	ldx _udata+U_DATA__U_PTAB
	ldab P_TAB__P_STATUS_OFFSET,x
	cmpb #P_RUNNING
	bne not_running
	; Set the punishment flag if we used our time up
	ldab #P_READY
	stab P_TAB__P_STATUS_OFFSET,x
	ldaa P_TAB__P_FLAGS_OFFSET,x
	orab #PFL_BATCH
	stab P_TAB__P_FLAGS_OFFSET,x
	;
	; At this point we are on the kernel stack and it all
	; looks like a syscall. We can now switch out. The
	; interrupt is effectively over and the new task will
	; set the flags accordingly. In time we will pop out of
	; plt_switchout again as if we had finished a syscall
	;
not_running:
	jsr _plt_switchout
	; We no longer want interrupts off when we run
	clr _udata+U_DATA__U_ININTERRUPT
	; We are not in a syscall
	clr _udata+U_DATA__U_INSYS
	; We are mapoped in memory
	jsr map_proc_always
	; Move to user stack before interrupt exit
	lds _udata+U_DATA__U_SYSCALL_SP
	ldaa _udata+U_DATA__U_CURSIG
	beq no_signals
	jmp rti_return

	.code

	.setcpu 6303

_set_cpu_type:
	clrb
	ldx #1
	xgdx		; appears to do nothing on the 6803
	tstb
	beq is_6803
	ldab #3		; 6303 feature set also present
	stab _sys_cpu_feat
is_6803:
	rts

	.setcpu 6803

	.data
_sys_cpu:
	.byte 2		; 6800 class CPU
_sys_cpu_feat:
	.byte 1		; 6800 with 6801/3 features
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
