#
;
;	Zilog Z8 low level code
;
;	A bit nicer than the Z8
;
;
#include "../cpu-super8/kernel-super8.def"

;
;	CPU setup and properties. For our purposes all Z8 is the same
;
		.data

.export _sys_cpu
.export _sys_cpu_feat
.export _sys_stubs

_sys_cpu:
	.byte 5		; Zilog Z8
_sys_cpu_feat:
	.byte 2

_sys_stubs:
	jp	unix_syscall_entry
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

.export deliver_signals_2
.export deliver_signals

		.common

callr0:
	push	r1
	push	r0
justret:
	ret

deliver_signals:
	lde	r3,U_DATA__U_CURSIG
	or	r3,r3
	jr	z, justret
deliver_signals_2:
	ldw	rr14,#U_DATA__U_SIGVEC
	ld	r2,r3
	add	r3,r3
	add	r15,r3
	adc	r14,#0
	clr	r3
	lde	r0,@rr14
	lde	@rr14,r3
	incw	rr14
	lde	r1,@rr14
	lde	@rr14,r3
	
	; r0,r1 is the vector, r2 is the signal, vector has been cleared
	ld	r3,r0
	or	r3,r1
	jr	z, raced

	; Build the return frame
	push	r2
	clr	r2
	push	r2

	clr	r2
	lde	U_DATA__U_CURSIG,r2

	ei
	call	callr0
	di
raced:
	ldw	rr14,#U_DATA__U_SYSCALL_SP
	ldw	rr2,216
	lde	@rr14,r2		; save sp
	ldepi	@rr15,r3
	ldw	2,#kstack_top
	ldw	216, 2

	clr	_int_disabled	; we keep this in register space as its asm only
	call	map_kernel_di
	call	_chksigs
	call	map_proc_always_di
	ldw	rr14,#U_DATA__U_SYSCALL_SP
	ldei	r14,@rr14		; restore sp
	lde	r1,@rr15
	ldw	216,14
	jr	deliver_signals

.export unix_syscall_entry

;
;	On entry our call frame looks like
;
;	ret to sycall stub helper
;	ret to syscall func itself
;	arguments
;
;	the caller expects r4-r11 to be preserved
;	on entry r3 is the call num
;	on exit r2,r3 is the return r0,r1 is error code
;
unix_syscall_entry:
	di
	lde	U_DATA__U_CALLNO,r3

	; Now block copy the args
	ldw	rr14,#U_DATA__U_ARGN

	ldw	rr12,216
	adc	r13,#4
	add	r12,#0

	ldei	r3,@rr12
	lde	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3
	ldei	r3,@rr12
	ldepi	@rr14,r3

	; We are now in kernel space
	ld	r3,#1
	lde	U_DATA__U_INSYS,r3
	
	; Switch stacks
	ldw	rr14,#U_DATA__U_SYSCALL_SP
	ldw	rr2,216
	lde	@rr14,r2
	ldepi	@rr14,r3
	ldw	216,#kstack_top

	;
	; Now map the kernel and call it
	;
	call	map_kernel_di
	ei
	call	_unix_syscall

	; r2,r3 is the return from unix_syscall

	;
	; Remember fork and execve don't necessarily return this way and fork
	; can do it twice
	;
	di
	call	map_proc_always
	clr	r3
	lde	U_DATA__U_INSYS,r3
	; Switch stack back
	ldw	rr14,#U_DATA__U_SYSCALL_SP
	ldei	r0,@rr14
	lde	r1,@rr14
	ldw	216,rr0

	; Get the return info
	ldw	rr14,#U_DATA__U_RETVAL
	ldei	r2,@rr14
	ldei	r3,@rr14		; into U_ERROR
	ldei	r0,@rr14
	lde	r1,@rr14

	lde	r12,U_DATA__U_CURSIG
	or	r12,r12
	jr	nz,via_signal
unix_return:
	ld	r12,r0
	or	r12,r1
	jr	z, not_error
	scf
	; Carry and errno in HL as expected
	jr	unix_pop
not_error:
	ccf
unix_pop:
	ei
	ret

via_signal:
	;
	; Stack the state (a signal doing a syscall will change the
	; U_DATA fields but we must return the old error/status)
	;
	push	r3
	push	r2
	push	r1
	push	r0
	;
	; And into the signal delivery path
	;
	call deliver_signals_2
	pop	r0
	pop	r1
	pop	r2
	pop	r3
	jr	unix_return

;
;	Called when execve() completes to transition to the user, as we
;	don't return from execve() via the syscall path
;
;
.export _doexec

_doexec:
	di
	call	map_proc_always
	; Get the argument
	pop	r0
	pop	r1
	pop	r2
	pop	r3

	; set up the stack
	ldw	rr14,#U_DATA__U_ISP
	ldei	r0,@rr14
	lde	r1,@rr14
	ldw	216,rr0

	; No longer in syscalls
	clr	r0
	lde	U_DATA__U_INSYS,r0
	ei
	; jump to the passed address
	push	r3
	push	r2
	ret
;
;	NULL trap. Must live in common space
;
;	FIXME: Rewrite 68000 style as a synchronous trap
;
.export null_handler

null_handler:
	lde	r3,U_DATA__U_INSYS
	or	r3,r3
	jr	nz, trap_illegal
	lde	r3,U_DATA__U_ININTERRUPT
	or	r3,r3
	jr	nz,trap_illegal
	ld	r3,#7
	clr	r2
	push	r2
	push	r3
	ldw	rr14,#U_DATA__U_PTAB
	ldei	r2,@rr14
	lde	r3,@rr14
	incw	rr2
	incw	rr2
	incw	rr2	; P_PID_OFFSET
	ldei	r0,@rr2
	lde	r1,@rr2
	push	r1
	push	r0
	ld	r3,#39
	clr	r2
	push	r3
	push	r2
	; signal
	call	unix_syscall_entry
	ld	r3,#0xff
	push	r3
	push	r3
	clr	r3	; exit(-1);
	call unix_syscall_entry
	; Never returns

trap_illegal:
	ldw	rr14,#illegalmsg
trapr14:
	call	outstring
	call	_plt_monitor

.export nmi_handler

nmi_handler:
	call	map_kernel_di
	ld	r14,#>nmimsg
	ld	r15,#<nmimsg
	jp	trapr14

illegalmsg:
	.ascii '[illegal]'
	.byte 0
nmimsg:
	.ascii '[NMI]'
	.byte 0

;
;	Interrupts are similar to Z80 but we have a lot less state
;	to store, and rather trickier juggling to get signals nice
;
.export interrupt_handler

reinterrupt:
	; Interrupt during interrupt but on alt stack (eg swap)
	ld	r0,#1
	ld	_int_disabled,r0

	call	_plt_reinterrupt
	srp	#MAINBANK
	iret

interrupt_handler:
	; This approach won't work if we allow reinterrupt - FIXME 
	srp	#INTBANK
	call	plt_interrupt_all

	lde	r3,U_DATA__U_ININTERRUPT
	or	r3,r3
	jr	nz, reinterrupt

	; Switch stacks
	ldw	istack_switched_sp,216
	ldw	216,#istack_top

	;
	; Map the kernel
	;
	call	map_save_kernel
	;
	; Set up state and enter kernel
	;
	ld	r3,#1
	lde	U_DATA__U_ININTERRUPT,r3
	ld	_int_disabled,r3

	call	_plt_interrupt

	;
	; Do we need to task switch ?
	;
	lde	r3,_need_resched
	or	r3,r3
	jr	nz,preemption
	;
	; Switch stacks back
	;
	call	map_restore
	ldw	216,istack_switched_sp
intout:
	clr	r3
	lde	U_DATA__U_ININTERRUPT,r3

	lde	r3,U_DATA__U_INSYS
	or	r3,r3
	jr	nz, interrupt_pop
	call	deliver_signals
	;
	; Restore registers and done
	;
interrupt_pop:
	clr	_int_disabled
	srp 	#MAINBANK
	iret

trap_signal:
	; r2,r3 is signal
	push	r3
	push	r2
	ldw	rr2,#U_DATA__U_PTAB
	push	r3
	push	r2
	call	_ssig
	incw	216
	incw	216
	incw	216
	incw	216
	ret

;
;	Now the scary stuff - preempting
;	
preemption:
	clr	r3
	lde	_need_resched,r3
	;
	;	Save our original stack in syscall_s
	;	Move to our kernel stack (free because we don't preempt
	;	in kernel
	;
	ldw	rr14,#U_DATA__U_SYSCALL_SP
	ld	r0,istack_switched_sp
	lde	@rr14,r0
	ld	r1,istack_switched_sp+1
	ldepi	@rr14,r1
	ldw	216,#kstack_top
	;
	;	Mark ourselves as in a system call
	;
	ld	r3,#1
	lde	U_DATA__U_INSYS,r3
	call	_chksigs
	ldw	rr14,#U_DATA__U_PTAB
	ldei	r2,@rr14
	lde	r3,@rr14
	lde	r0,@rr2
	cp	r0,#P_RUNNING
	jr	nz,not_running
	ld	r0,#P_READY
	lde	@rr2,r0
	;
	;	Punish the process for using all of its time.
	;
	incw	rr2	; flags
	lde	r0,@rr2
	or	r0,#PFL_BATCH
	lde	@rr2,r0

not_running:
	;
	;	We will disappear into this and reappear somewhere else. In
	;	time we will reappear here
	;
	call	_plt_switchout
	;
	;	We are back in the land of the living so no longer in
	;	syscall or interrupt state
	;
	clr	r3
	lde	U_DATA__U_ININTERRUPT,r3
	lde	U_DATA__U_INSYS,r3
	;
	;	Get our mapping back
	;
	call map_proc_always_di
	;
	;	And our stack
	;
	ldw	rr14,#U_DATA__U_SYSCALL_SP
	ldei	r2,@rr14
	lde	r3,@rr14
	ldw	216,rr2

	lde	r3,U_DATA__U_CURSIG
	or	r3,r3
	jr	nz, nosigcall
	call	deliver_signals_2
nosigcall:
	jp interrupt_pop

;
;	Debug code
;
.export outstring

outstring:
	ldei	r3,@rr14
	or	r3,r3
	jr	z, outsdone
	call	outchar
	jr	outstring
outsdone:
	ret

.export outstringhex

outstringhex:
	ldei	r3,@rr14
	or	r3,r3
	jr	z,outsdone
	call	outcharhex
	ld	r3,#' '
	call	outchar
	jr	outstringhex

.export outnewline

outnewline:
	ld	r3,#0x0d
	call	outchar
	ld	r3,#0x0a
	jp	outchar

.export outr2

outr2:
	push	r3
	ld	r3,r2
	call outcharhex
	pop	r3
	call outcharhex
	ret

.export outr14

outr14:
	push	r3
	ld	r3,r14
	call outcharhex
	ld	r3,r15
	call outcharhex
	pop	r3
	ret

.export outcharhex

outcharhex:
	; Print hex of r3
	push	252		; flags
	push	r3
	swap	r3
	call	outnibble
	pop	r3
	push	r3
	call	outnibble
	pop	r3
	pop	252
	ret

outnibble:
	and	r3,#0x0f
	cp	r3,#10
	jr	c,numeral
	add	r3,#7
numeral:
	add	r3,#0x30		; '0'
	jp	outchar


.export ___hard_ei

___hard_ei:
	clr	_int_disabled
	ei
	ret

.export ___hard_di

___hard_di:
	ld	r3,_int_disabled
	di
	ld	_int_disabled,#1
	ret

.export ___hard_irqrestore

___hard_irqrestore:
	ldw	rr14,216
	lde	r3,2(rr14)
	ld	_int_disabled,r3
	di
	cp	_int_disabled,#0
	jr	nz, diret
	ei
	; FIXME: stack cleanup - ditto on Z8 one
diret:
	ret

;
;	We have no variants to care about (eZ8 is quite different)
;
.export _cpu_detect

_cpu_detect:
	ret

;
;	We need to worry about bits of this in interrupt save and restore
;
;
;	I/O helpers. ACK lacks inline assembly which is annoying. Doubly so
;	because 8080/8085 can only access variable ports by self modifying
;	code.. which is fun with interrupts. Bletch;
;
;	For speed critical cases you need asm stubs, for the others these
;	will do.
;
;	TODO: I/O mapping is board specific so this belongs in the platform
;
.export _in
.export _out

;
;	The production board uses upper 32K of the data space as I/O
;	whenever port 2 pin 6 is low
;
;	FIXME ; pull from monitor move to platform fix cleanup
;
_in:
	ld	r15,#2
	call	__gargr2
	ld	r12,2
	push	r12
	and	2,#0xBF		; I/O on
	ld	r2,#0xFF	; I/O is at FFxx
	lde	r3,@rr2
	pop	r12		; restore I/O (might be on in an intr)
	clr	r2
	ret

_out:
	ld	r15,#2
	call	__gargr4	; get both args into r0-r1/r2-r3
	ld	r12,2
	and	2,#0xBF		; I/O on
	push	r12
	ld	r0,#0xFF
	lde	@rr0,r3
	pop	2
	ret

.export _set_cpu_type

_set_cpu_type:
	ret

	.export _memcpy
	.export _memset
	.export _strlen
	.export _swab
	.export _swab32

;
;	Arg cleanup FIXME here and Z8
;
_memcpy:
	ld r15,#4
	call __gargr4
	ld r15,#6
	call __garg12r2
	; r12 is length
	; r0/r1 dest
	; r2/r3 src
	incw r12
	push r1
	push r0
	decw rr0
	jr mcnext
mcloop:
	ldei r14,@rr2
	ldepi @rr0,r14
mcnext:
	decw rr12
	jr nz,mcloop
	pop r2
	pop r3
	ret
	
_memset:
	ld r15,#4
	call __gargr4
	ld r15,#6
	call __garg12r2
	; r12 is length
	; r0/r1 ptr
	push r1
	push r0
	; r3 value
	incw rr12
	jr msnext
msloop:
	lde @rr0,r3
	incw rr0
msnext:
	decw rr12
	jr nz, msloop
	pop r2
	pop r3
	ret

_strlen:
	ld r15,#2
	call __garg12r2
	; r1,r13 is now ptr
	clr r2
	clr r3
count:
	ldei r0,@rr12
	or r0,r0
	jr z,strlen_done
	incw rr2
	jr count
strlen_done:
	ret

_swab:
	ld r15,#2
	call __gargr2
	ld r0,r3
	ld r3,r2
	ld r2,r0
	ret

_swab32:
	ld r15,#2
	call __gargr4
	ld r12,r3
	ld r3,r0
	ld r0,r12
	ld r12,r2
	ld r2,r1
	ld r1,r12
	ret
