;
;	Core low level code for the 807x processors
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

	.export preemption

	.export	__tmp
	.export __tmp2
	.export __hireg

#include "../build/kernel.def"
#include "../cpu-8070/kernel-8070.def"

	.dp

__tmp:
	.word	0
__tmp2:
	.word	0
__hireg:
	.word	0

	.code

_di:
	ld	a,s
	and	s,=0xFE
	ret

_ei:
	or	s,=1
	ret

_irqrestore:
	ld	a,2,p1
	ld	s,a
	ret		; clobbered anyway

_swab:
	ld	ea,2,p1
	xch	a,e
	ret

	.common

outnewline:
	ld	a,=0x0d
	jsr	outchar
	ld	a,=0x0a
	jmp	outchar


outcharhex:
	push	a
	sr	a
	sr	a
	sr	a
	sr	a
	jsr	outnibble
	ld	a,0,p1
	jsr	outnibble
	pop	a
outsdone:
	ret

outnibble:
	and	a,=0x0F
	ld	e,a
	sub	a,=0x0A
	ld	a,s
	bp	outh2
	ld	a,e
	add	a,=0x30
	jmp	outchar
outh2:
	ld	a,e
	add	a,=0x37
	jmp outchar

outstring:
	ld	a,@1,p3
	bz	outsdone
	jsr	outchar
	bra	outstring

outp3:
	push	ea
	ld	ea,p3
	xch	e,a
	jsr	outcharhex
	ld	ea,p3
	jsr	outcharhex
	pop	ea
	ret

outea:
	push	ea
	xch	e,a
	jsr	outcharhex
	ld	ea,0,p1
	jsr	outcharhex
	pop	ea
	ret


;
;	On entry our frame is
;	0	return address of syscall helper
;	2	return address of call
;	4	first argument
;	6	second argument
;	etc
;
;	P1 is stack (user)
;	P2/P3 can be trashed
;	A is call number
;
;	Returns P2 = result, EA = error (must be < 256)
;
unix_syscall_entry:
	ld	p3,=_udata
	st	a,U_DATA__U_CALLNO,p3
	ld	ea,p1
	st	ea,U_DATA__U_SYSCALL_SP,p3
	ld	ea,4,p1
	st	ea,U_DATA__U_ARGN,p3
	ld	ea,6,p1
	st	ea,U_DATA__U_ARGN+2,p3
	ld	ea,6,p1
	st	ea,U_DATA__U_ARGN+4,p3
	ld	ea,6,p1
	st	ea,U_DATA__U_ARGN+6,p3

	ld	a,=1
	st	a,U_DATA__U_INSYS,p3	; we may want to use udata-> tricks ?
	ld	p1,=kstack_top
	jsr	map_kernel_di		; no-op in pure banked
	and	s,=0xfe
	jsr	_unix_syscall
	or	s,=0x01
	ld	a,=0
	ld	p3,=_udata
	st	a,U_DATA__U_INSYS,p3
	jsr	map_proc_always	; no-op in pure banked
	ld	ea,U_DATA__U_SYSCALL_SP,p3
	ld	p1,ea

do_signal:
	ld	ea,=0
	ld	a,U_DATA__U_CURSIG,p3
	bz	no_signal
	sl	ea
	add	ea,=_udata+U_DATA__U_SIGVEC
	ld	p2,ea
	ld	ea,0,p2			; vector
	bz	no_signal
	ld	t,ea			; vector safed in T
	ld	ea,=0
	st	ea,0,p2			; clear vector

	ld	a, U_DATA__U_CURSIG,p3
	ld	e,a			; save signal number in e
	ld	a,=0
	st	a,U_DATA__U_CURSIG,p3	; clear it
	xch	a,e
	
	; Arguments to signal handler
	push	ea			; signal number
	; Return
	ld	ea,=unwind
	push	ea
	; Unwind for signal handlers
	ld	p3,=PROGLOAD+16
	ld	ea,0,p3
	push	ea
	ret
unwind:
	pop	ea
no_signal:
	ld	p3,=_udata
	ld	ea,U_DATA__U_RETVAL,p3
	ld	p2,ea
	ld	ea,U_DATA__U_ERROR,p3
	ret

_doexec:
	ld	ea,2,p1
	and	s,=0xfe
	ld	t,ea
	jsr	map_proc_always
	ld	p3,=_udata
	ld	ea,U_DATA__U_ISP,p3
	ld	p1,ea
	ld	ea,=0
	st	a,U_DATA__U_INSYS,p3
	or	s,=1
	;	Now into user code with the needed info
	ld	ea,t
	push	ea
	;	We hardcode the user DP space but pass it anyway
	;	in case it ever becomes dynamic
	ld	p3,=0xFFF0
	ret

trap_handler:
	ld	p3,=_udata
	ld	a,U_DATA__U_INSYS,p3
	bnz	trap_illegal
	ld	a,U_DATA__U_ININTERRUPT,p3
	bnz	trap_illegal
	ld	ea,=9
	push	ea
	ld	ea,U_DATA__U_PTAB,p3
	ld	p2,ea
	ld	ea,P_TAB__P_PID_OFFSET,p2
	push	ea
	ld	a,=35
	push	ea	; dummy return path
	jmp	unix_syscall_entry

trap_illegal:
	ld	p3,=illegalmsg
trapx:
	jsr	outstring
	jsr	_plt_monitor

nmi_handler:
	jsr map_kernel_di
	ld	p3,=nmimsg
	bra	trapx

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
	push	ea
	ld	a,s
	push	a
	ld	ea,t
	push	ea
	push	p2
	push	p3
	ld	ea,p1
	ld	p2,=istack_switched_sp
	st	ea,0,p2

	ld	p1,=istack_top

	jsr map_save_kernel

	ld	a,=1
	ld	p3,=_udata
	st	a,U_DATA__U_ININTERRUPT,p3

	; Save the C direct page values so we can re-enter
	; If we turn on register variables we will need to save them too
	ld	ea,:__tmp
	push	ea
	ld	ea,:__tmp2
	push	ea
	ld	ea,:__hireg
	push	ea
	jsr	_plt_interrupt
	; Restore the C direct page
	pop	ea
	st	ea,:__hireg
	pop	ea
	st	ea,:__tmp2
	pop	ea
	st	ea,:__tmp
	; If we interrupted kernel mode then we just put the map back and
	; return
	ld	p3,=_udata
	ld	a,U_DATA__U_INSYS,p3
	bnz	iretk
	; If we had to reschedule it's more complicated
	ld	p2,=_need_resched
	ld	a,0,p2
	bnz	preemption
	; Put the process memory map back
	jsr	map_proc_always
	; Put the process stack back
	ld	p2,=istack_switched_sp
	ld	ea,0,p2
	ld	p1,ea
	; We are no longer on the istack or in an interrupt
	ld	a,=0
	st	a,U_DATA__U_ININTERRUPT,p3
	; Don't deliver signals if we were in a system call
	ld	a,U_DATA__U_INSYS,p3
	bnz	no_signals
	;	TODO stack a signal frame that unwinds into no_signals
	;	if needed
	;	can we use do_return somehow ??
no_signals:
	pop	p3
	pop	p2
	pop	ea
	ld	ea,t
	pop	a
	ld	s,a
	pop	ea
	ret
	;
	; Return to kernel mode
	; We don't check signals in this case and we use map_restore
	;
iretk:
	jsr	map_restore
	ld	p2,=istack_switched_sp
	ld	ea,p2
;TODO	ld	p2,
	ld	a,=0
	st	a,U_DATA__U_ININTERRUPT,p3
	bra	no_signals


;
;	Task switch due to an interrupt.
;
preemption:
	; Acknowledge the request
	ld	a,=0
	st	a,0,p2		; clear need_resched

	; Copy the user stack save to the syscall save
	ld	p2,=istack_switched_sp
	ld	ea,0,p2
	st 	ea,U_DATA__U_SYSCALL_SP,p3
	; Move onto our process kernel stack
	; We can't stay on the istack as it'll be gone when
	; we return.
	ld	p1,=kstack_top
	; Make it look like we are in a syscall
	ld	a,=1
	st	a,U_DATA__U_INSYS,p3
	; Update our signals
	jsr	_chksigs
	; Correct our process state
	ld	ea,U_DATA__U_PTAB,p3
	ld	p2,ea
	ld	a,P_TAB__P_STATUS_OFFSET,p2
	sub	a,=P_RUNNING
	bnz	not_running
	; Set the punishment flag if we used our time up
	ld	a,=P_READY
	st	a,P_TAB__P_STATUS_OFFSET,p2
	ld	a,P_TAB__P_FLAGS_OFFSET,p2
	or	a,=PFL_BATCH
	st	a,P_TAB__P_FLAGS_OFFSET,p2
	;
	; At this point we are on the kernel stack and it all
	; looks like a syscall. We can now switch out. The
	; interrupt is effectively over and the new task will
	; set the flags accordingly. In time we will pop out of
	; plt_switchout again as if we had finished a syscall
	;
not_running:
	jsr	_plt_switchout
	; We no longer want interrupts off when we run
	ld	a,=0
	ld	p3,=_udata
	st	a,U_DATA__U_ININTERRUPT,p3
	; We are not in a syscall
	st	a,U_DATA__U_INSYS,p3
	; We are mapoped in memory
	jsr	map_proc_always
	; Move to user stack before interrupt exit
	ld	ea,U_DATA__U_SYSCALL_SP,p3
	ld	p1,ea
	ld	a,U_DATA__U_CURSIG,p3
	bz	no_signals
	; TODO as per interrupt code
;	jmp	rti_return

	.code

; We can probable tell 807x apart but why bother ?
_set_cpu_type:
	ret

	.data
_sys_cpu:
	.byte 10	; 8070
_sys_cpu_feat:
	.byte 0		; Generic
_sys_stubs:
	jmp	unix_syscall_entry
	ret

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

;
;	C helpers
;

	.export _strlen

_strlen:
	ld	ea,2,p1
	ld	p2,ea

next:
	ld	a,@1,p2
	bnz	next
	ld	ea,p2
	sub	ea,2,p1
	ret

	.export _memcpy

_memcpy:
	ld	ea,2,p1
	ld	p2,ea		; dest
	ld	ea,4,p1
	ld	p3,ea		; src
	ld	ea,6,p1		; len
	add	ea,=0x0100
	add	a,=0x01
	st	ea,:__tmp
	bra	mcnext
mcloop:
	ld	a,@1,p3
	st	a,@1,p2
mcnext:
	dld	a,:__tmp
	bnz	mcloop
	dld	a,:__tmp+1
	bnz	mcloop
	ld	ea,2,p1
	ret

	.export _memset

_memset:
	ld	ea,2,p1
	ld	p2,ea		; dest
	ld	ea,4,p1
	ld	t,ea		; pattern
	ld	ea,6,p1		; len
	add	ea,=0x0100
	add	a,=0x01
	st	ea,:__tmp
	bra	msnext
msloop:
	ld	ea,t
	st	a,@1,p2
msnext:
	dld	a,:__tmp
	bnz	msloop
	dld	a,:__tmp+1
	bnz	msloop
	ld	ea,2,p1
	ret

