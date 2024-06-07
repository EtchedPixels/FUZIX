;
;	We use the same low level code. There is no particular reason to use
;	xgdx and friends here so 6803 and 6303 can share
;
	.export _swab
	.export _doexec
	.export _sys_cpu
	.export _sys_stubs
	.export _sys_cpu_feat
	.export _set_cpu_type
	.export unix_syscall_entry
	.export interrupt_handler

	.export outnewline
	.export outcharhex
	.export outstring

	.export preemption

	.export deliver_signals

	.setcpu 6

#include "../build/kernel.def"
#include "kernel-wrx6.def"

	.code

_swab:
	ldb	2(s)
	xfrb	bh,al
	xfrb	bl,bh
	xfrb	al,bl
	rsr

	.common

outnewline:
	lda	13
	jsr	outchar
	lda	10
	jsr	outchar
	rsr

outcharhex:
	st	a,(-s)
	rrr	a,4
	jsr	outnibble
	ld	a,(s+)
	jsr	outnibble
	rsr

outnibble:
	ldb	0x0F
	and	b,a
	ldb	10
	sab
	bl	outh2
	ldb	7
	add	b,a
outh2:	ldb	30
	add	b,a
	jsr outchar
outsdone:
	rsr

outstring:
	ldab	(x+)
	bz	outsdone
	jsr	outchar
	bra	outstring

deliver_signals:
	ldbb	(_udata+U_DATA__U_CURSIG)
	bnz	deliver_signals_2
signal_raced:
	rsr
deliver_signals_2:
	ldx	_udata+U_DATA__U_SIGVEC
	add	b,x
	add	b,x
	lda	(x)
	bz	signal_raced
	;	Stack the signal number
	stb	(-s)
	;	Stack the vector
	sta	(-s)
	cla
	sta	(x)
	stab	(_udata+U_DATA__U_CURSIG)
	; 	Signal helper
	;	Will need rewriting if this thing has a protected mode
	jsr	(PROGLOAD+16)

	; discard the vector
	ldb	(s+)
	ldb	(s+)
	xfr	s,a
	sta	(_udata+U_DATA__U_SYSCALL_SP)
	lda	kstack_top
	xas
	jsr	map_kernel_di
	jsr	_chksigs
	jsr	map_proc_always_di
	lda	(_udata+U_DATA__U_SYSCALL_SP)
	xas
	bra	deliver_signals
	

;
;	Top of stack is the old X, above it the old X for the sycall itself
;	above that the arguments
;
;
unix_syscall_entry:
	stx	(-s)
	xfr	s,x
	stx	(_udata+U_DATA__U_SYSCALL_SP)

	inx
	inx
	inx
	inx

	stab	( _udata+U_DATA__U_CALLNO)
	lda	(x+)
	sta	(_udata+U_DATA__U_ARGN)
	lda	(x+)
	sta	(_udata+U_DATA__U_ARGN+2)
	lda	(x+)
	sta	(_udata+U_DATA__U_ARGN+4)
	lda	(x)
	sta	(_udata+U_DATA__U_ARGN+6)
	ldab	1
	stab 	(_udata+U_DATA__U_INSYS)	; we may want to use udata-> tricks ?
	lda	kstack_top
	xas
	jsr	map_kernel_di		; no-op in pure banked
	di
	jsr	_unix_syscall
	ei
	clab
	stab	(_udata+U_DATA__U_INSYS)
	jsr	map_proc_always	; no-op in pure banked
	lda	(_udata+U_DATA__U_SYSCALL_SP)
	xas
	lda	(_udata+U_DATA__U_RETVAL)
	ldx	(_udata+U_DATA__U_ERROR)
	ldab	(_udata+U_DATA__U_CURSIG)
	bnz	via_signal
	xfr	x,y
	ldx	(s+)
	rsr
via_signal:
	sta	(-s)
	stx	(-s)
	jsr	deliver_signals_2
	ldx	(s+)
	xfr	x,y
	lda	(s+)
	ldx	(s+)
	rsr

_doexec:
	ldx	4(s)
	di
	jsr	map_proc_always
	lda	(_udata+U_DATA__U_ISP)
	xas
	clab
	stab	(_udata+U_DATA__U_INSYS)
	ei
	stx	(-s)
	jmp	(x)

;
;	This occurs on a different ipl level and it's all quite weird
;
interrupt_handler:
	jsr	map_save_kernel

	lda 	1
	sta	(_udata+U_DATA__U_ININTERRUPT)

	jsr	_plt_interrupt

	ldab	(_udata+U_DATA__U_INSYS)
	bnz	iretk
	clab
	ldab	(_need_resched)
	bnz	preemption
	jsr	map_proc_always
	clab
	stab	(_udata+U_DATA__U_ININTERRUPT)
	ldab	(_udata+U_DATA__U_INSYS)
	bnz	no_signals
	;
	;	TODO: this is much more complicated. We will need to push
	;	a frame onto the user stack rather than do this the
	;	way many 8bit systems do. In fact it's going to be like 68K
	;
	jsr	deliver_signals
no_signals:
	ri
	;	Because this is a co-routine switch we need to loop
	jmp	interrupt_handler

	;
	; Return to kernel mode
	; We don't check signals in this case and we use map_restore
	;
iretk:
	jsr	map_kernel_restore
	clab
	stab	(_udata+U_DATA__U_ININTERRUPT)
	stab	(_udata+U_DATA__U_INSYS)
	ri
	jmp 	interrupt_handler


;
;	This will be fun - punt for now
;
preemption:
	clab
	stab	(_need_resched)
#if 0
	; Save the stack pointer across
	ldb	(istack_switched_sp)
	stb	(_udata+U_DATA__U_SYSCALL_SP)
	
	lda	kstack_top
	xas
	ldab	1
	stab	(_udata+U_DATA__U_INSYS)
	jsr	_chksigs
	ldx	(_udata+U_DATA__U_PTAB)
	ldab	P_TAB__P_STATUS_OFFSET(x)
	ldbb	P_RUNNING
	sabb
	bnz	not_running
	ldab	P_TAB__P_STATUS_OFFSET(x)
	ldb	P_READY
	stab	P_TAB__P_STATUS_OFFSET(x)
	ldab	P_TAB__P_FLAGS_OFFSET(x)
	ldbb	PFL_BATCH
	ori	b,a
	stab	P_TAB__P_FLAGS_OFFSET(x)
not_running:
	jsr	_plt_switchout
	clab
	stab	_udata+U_DATA__U_ININTERRUPT
	stab	_udata+U_DATA__U_INSYS
	jsr	map_proc_always
	lda	(_udata+U_DATA__U_SYSCALL_SP)
	xas
	ldaa	(_udata+U_DATA__U_CURSIG)
	bz	no_signals
	jsr	deliver_signals_2
#endif
	ri
	jmp 	interrupt_handler

	.code

;

_set_cpu_type:
	ldab	1
	stab	(_sys_cpu_feat)
	rsr

	.data
_sys_cpu:
	.byte 2		; CDI/EE200/Warrex CPU
_sys_cpu_feat:
	.byte 1		; CPU 6
_sys_stubs:
	jmp	unix_syscall_entry
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
