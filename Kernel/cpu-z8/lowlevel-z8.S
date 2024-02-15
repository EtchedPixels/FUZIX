#
;
;	Zilog Z8 low level code
;
;	Much the same as the Z80 code except we don't provide in and out
;	helpers because it's impossible to make them re-entrant for any port
;
#include "../cpu-z8/kernel-z8.def"

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
	.byte 0

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
	ld	r14,#>U_DATA__U_CURSIG
	ld	r15,#<U_DATA__U_CURSIG
	lde	r3,@rr14
	or	r3,r3
	jr	z, justret
deliver_signals_2:
	ld	r14,#>U_DATA__U_SIGVEC
	ld	r15,#<U_DATA__U_SIGVEC
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

	ld	r14,#>U_DATA__U_CURSIG
	ld	r15,#<U_DATA__U_CURSIG
	clr	r2
	lde	@rr14,r2

	ei
	call	callr0
	di
raced:
	ld	r14,#>U_DATA__U_SYSCALL_SP
	ld	r15,#<U_DATA__U_SYSCALL_SP
	ld	r2,254
	ld	r3,255
	lde	@rr14,r2		; save sp
	incw	rr14
	lde	@rr15,r3
	ld	254,#>kstack_top
	ld	255,#<kstack_top

	clr	_int_disabled	; we keep this in register space as its asm only
	call	map_kernel_di
	call	_chksigs
	call	map_proc_always_di
	ld	r14,#>U_DATA__U_SYSCALL_SP
	ld	r15,#<U_DATA__U_SYSCALL_SP
	lde	r04,@rr14		; restore sp
	incw	rr14
	lde	r1,@rr15
	ld	254,14
	ld	255,15
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
	ld	r14,#>U_DATA__U_CALLNO
	ld	r15,#<U_DATA__U_CALLNO
	lde	@rr14,r3	; save call number

	; Now block copy the args
	ld	r14,#>U_DATA__U_ARGN
	ld	r15,#<U_DATA__U_ARGN

	ld	r12,254
	ld	r13,255
	adc	r13,#4
	add	r12,#0

	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3
	lde	r3,@rr12
	lde	@rr14,r3

	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	; We are now in kernel space
	ld	r3,#1
	lde	@rr14,r3
	
	; Switch stacks
	ld	r14,#>U_DATA__U_SYSCALL_SP
	ld	r15,#<U_DATA__U_SYSCALL_SP
	ld	r2,254
	ld	r3,255
	lde	@rr14,r2
	incw	rr14
	lde	@rr15,r3
	ld	254,#>kstack_top
	ld	255,#<kstack_top

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
	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	lde	@rr14,r3
	; Switch stack back
	ld	r14,#>U_DATA__U_SYSCALL_SP
	ld	r15,#<U_DATA__U_SYSCALL_SP
	lde	r0,@rr14
	incw	rr14
	lde	r1,@rr14
	ld	254,r0
	ld	255,r1

	; Get the return info
	ld	r14,#>U_DATA__U_RETVAL
	ld	r15,#<U_DATA__U_RETVAL
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	incw	rr14		; into U_ERROR
	lde	r0,@rr14
	incw	rr14
	lde	r1,@rr14

	ld	r14,#>U_DATA__U_CURSIG
	ld	r15,#<U_DATA__U_CURSIG
	lde	r12,@rr14
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
	pop	r0
	pop	r2
	pop	r3

	; set up the stack
	ld	r14,#>U_DATA__U_ISP
	ld	r15,#<U_DATA__U_ISP
	lde	r0,@rr14
	ld	254,r0
	incw	rr14
	lde	r0,@rr14
	ld	255,r0

	; No longer in syscalls
	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	ld	r0,#0
	lde	@rr14,r0
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
	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	lde	r3,@rr14
	or	r3,r3
	jr	nz, trap_illegal
	ld	r14,#>U_DATA__U_ININTERRUPT
	ld	r15,#<U_DATA__U_ININTERRUPT
	lde	r3,@rr14
	or	r3,r3
	jr	nz,trap_illegal
	ld	r3,#7
	clr	r2
	push	r2
	push	r3
	ld	r14,#>U_DATA__U_PTAB
	ld	r15,#<U_DATA__U_PTAB
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	incw	rr2
	incw	rr2
	incw	rr2	; P_PID_OFFSET
	lde	r0,@rr2
	incw	rr2
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
	ld	r14,#>illegalmsg
	ld	r15,#<illegalmsg
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
	srp	#INTBANK
	call	plt_interrupt_all

	ld	r14,#>U_DATA__U_ININTERRUPT
	ld	r15,#<U_DATA__U_ININTERRUPT
	lde	r3,@rr14
	or	r3,r3
	jr	nz, reinterrupt

	; Switch stacks
	ld	istack_switched_sp,254
	ld	istack_switched_sp+1,255
	ld	254,#>istack_top
	ld	255,#<istack_top

	;
	; Map the kernel
	;
	call	map_save_kernel
	;
	; Set up state and enter kernel
	;
	ld	r14,#>U_DATA__U_ININTERRUPT
	ld	r15,#<U_DATA__U_ININTERRUPT
	ld	r3,#1
	lde	@rr14,r3
	ld	_int_disabled,r3

	call	_plt_interrupt

	;
	; Do we need to task switch ?
	;
	ld	r14,#>_need_resched
	ld	r15,#<_need_resched
	lde	r3,@rr14
	or	r3,r3
	jr	nz,preemption
	;
	; Switch stacks back
	;
	call	map_restore
	ld	254,istack_switched_sp
	ld	255,istack_switched_sp+1
intout:
	ld	r14,#>U_DATA__U_ININTERRUPT
	ld	r15,#<U_DATA__U_ININTERRUPT
	clr	r3
	lde	@rr14,r3

	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	lde	r3,@rr14
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
	ld	r2,#>U_DATA__U_PTAB
	ld	r3,#<U_DATA__U_PTAB
	push	r3
	push	r2
	call	_ssig
	adc	255,#4
	adc	254,#0
	ret

;
;	Now the scary stuff - preempting
;	
preemption:
	ld	r14,#>_need_resched
	ld	r15,#<_need_resched
	clr	r3
	lde	@rr14,r3
	;
	;	Save our original stack in syscall_s
	;	Move to our kernel stack (free because we don't preempt
	;	in kernel
	;
	ld	r14,#>U_DATA__U_SYSCALL_SP
	ld	r15,#<U_DATA__U_SYSCALL_SP
	ld	r0,istack_switched_sp
	lde	@rr14,r0
	incw	rr14
	ld	r1,istack_switched_sp+1
	lde	@rr14,r1
	ld	254,#>kstack_top
	ld	255,#<kstack_top
	;
	;	Mark ourselves as in a system call
	;
	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	ld	r3,#1
	lde	@rr14,r3
	call	_chksigs
	ld	r14,#>U_DATA__U_PTAB
	ld	r15,#<U_DATA__U_PTAB
	lde	r2,@rr14
	incw	rr14
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
	ld	r14,#>U_DATA__U_ININTERRUPT
	ld	r15,#<U_DATA__U_ININTERRUPT
	lde	@rr14,r3
	ld	r14,#>U_DATA__U_INSYS
	ld	r15,#<U_DATA__U_INSYS
	lde	@rr14,r3
	;
	;	Get our mapping back
	;
	call map_proc_always_di
	;
	;	And our stack
	;
	ld	r14,#>U_DATA__U_SYSCALL_SP
	ld	r14,#<U_DATA__U_SYSCALL_SP
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	ld	254,r2
	ld	255,r3

	ld	r14,#>U_DATA__U_CURSIG
	ld	r15,#<U_DATA__U_CURSIG
	lde	r3,@rr14
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
	lde	r3,@rr14
	or	r3,r3
	jr	z, outsdone
	call	outchar
	incw	rr14
	jr	outstring
outsdone:
	ret

.export outstringhex

outstringhex:
	lde	r3,@rr14
	or	r3,r3
	jr	z,outsdone
	call	outcharhex
	ld	r3,#' '
	call	outchar
	incw	rr14
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
	ld	r14,254
	ld	r15,255
	incw	r14
	incw	r14
	lde	r3,@rr14
	ld	_int_disabled,r3
	di
	cp	_int_disabled,#0
	jr	nz, diret
	ei
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
	jr mcnext
mcloop:
	lde r14,@rr2
	lde @rr0,r14
	incw rr0
	incw rr2
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
	incw r12
	jr msnext
msloop:
	lde @rr0,r3
	incw rr0
msnext:
	decw rr2
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
	lde r0,@rr12
	jr z,strlen_done
	incw rr12
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
