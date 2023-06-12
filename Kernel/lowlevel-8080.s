#
!
!	8080 low level code
!
!	Much the same as the Z80 code except we don't provide in and out
!	helpers because it's impossible to make them re-entrant for any port
!

#include "kernel-8080.def"

.sect .data

_sys_cpu:
	.data1 1		! 8080 family
_sys_cpu_feat:
	.data1 0		! No features

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

.sect .common

deliver_signals:
	lda U_DATA__U_CURSIG
	ora a
	rz
deliver_signals_2:
	mov l,a
	mvi h,0
	dad h
	lxi d,U_DATA__U_SIGVEC
	dad d
	mov e,m
	inx h
	mov d,m


	! Build the return frame
	lxi b,signal_return
	push b

	mov c,a		! save the signal number to pass into the helper

	xra a
	sta U_DATA__U_CURSIG
	!
	!	Do we need to zero check de here ?
	!
	mov a,d
	ora e
	jz signal_return		! raced

	!
	!	Off we go. DE = vector B = signal
	!
	!	FIXME: if we ever have 8080 binaries with different load
	!	addresses we will need to fix this
	!
	EI
	lhld PROGLOAD+16		! signal vector
	pchl
signal_return:
	DI
	lxi h,0
	dad sp
	shld U_DATA__U_SYSCALL_SP
	lxi sp,kstack_top
	mvi a,1
	sta _int_disabled
	call map_kernel_di
	call _chksigs
	call map_process_always_di
	lhld U_DATA__U_SYSCALL_SP
	sphl
	jmp deliver_signals

.define unix_syscall_entry

unix_syscall_entry:
	push b		! Must preserve the frame pointer
	lxi h,6		! Find arguments on stack frame

	dad sp
	sta U_DATA__U_CALLNO
	! Oh for LDIR
	! Unroll this for speed. Syscall arguments into constant locations
	mov a,m
	sta U_DATA__U_ARGN
	inx h
	mov a,m
	sta U_DATA__U_ARGN+1
	inx h
	mov a,m
	sta U_DATA__U_ARGN+2
	inx h
	mov a,m
	sta U_DATA__U_ARGN+3
	inx h
	mov a,m
	sta U_DATA__U_ARGN+4
	inx h
	mov a,m
	sta U_DATA__U_ARGN+5
	inx h
	mov a,m
	sta U_DATA__U_ARGN+6
	inx h
	mov a,m
	sta U_DATA__U_ARGN+7
	DI
	! We are now in kernel space
	mvi a,1
	sta U_DATA__U_INSYS
	! Switch stacks
	! On 8080 this is a bit more long winded as we have to go via HL
	lxi h,0
	dad sp
	shld U_DATA__U_SYSCALL_SP
	lxi sp, kstack_top
	!
	! Now map the kernel and call it
	!
	call map_kernel_di
	EI
	call _unix_syscall
	xchg
	!
	! Remember fork and execve don't necessarily return this way and fork
	! can do it twice
	!
	DI
	call map_process_always
	xra a
	sta U_DATA__U_INSYS
	! Switch stack back
	lhld U_DATA__U_SYSCALL_SP
	sphl
	lhld U_DATA__U_RETVAL
	xchg
	lhld U_DATA__U_ERROR
	!
	! Signal check
	!
	lda U_DATA__U_CURSIG
	ora a
	jnz via_signal
unix_return:
	mov a,h
	ora l
	jz not_error
	stc
	! Carry and errno in HL as expected
	jmp unix_pop
not_error:
	! Retval in HL as the Z80 kernel returns it
	xchg
unix_pop:
	pop b
	! ret must directly follow the ei
	EI
	ret
via_signal:
	!
	! Stack the state (a signal doing a syscall will change the
	! U_DATA fields but we must return the old error/status)
	!
	lhld U_DATA__U_ERROR
	push h
	lhld U_DATA__U_RETVAL
	push h
	!
	! And into the signal delivery path
	!
	call deliver_signals_2
	pop d
	pop h
	jmp unix_return

!
!	Called when execve() completes to transition to the user, as we
!	don't return from execve() via the syscall path
!
!
.define _doexec

_doexec:
	DI
	call map_process_always
	pop b
	pop d
	lhld U_DATA__U_ISP
	sphl
	xra a
	sta U_DATA__U_INSYS
	xchg
	mov d,h
	mvi e,0
	EI
	pchl
!
!	NULL trap. Must live in common space 
!
!	FIXME: Rewrite 68000 style as a synchronous trap
!
.define null_handler

null_handler:
	lda U_DATA__U_INSYS
	ora a
	jnz trap_illegal
	lda U_DATA__U_ININTERRUPT
	ora a
	jnz trap_illegal
	lxi h,7
	push h
	lhld U_DATA__U_PTAB
	lxi d,P_TAB__P_PID_OFFSET
	dad d
	mov e,m
	inx h
	mov d,m
	push d
	lxi h,39
	push h
	call unix_syscall_entry
	lxi h,0xffff			! exit -1
	push h
	dcx h
	push h
	call unix_syscall_entry
	! Never returns

trap_illegal:
	lxi h,illegalmsg
traphl:
	call outstring
	call _plt_monitor

.define nmi_handler

nmi_handler:
	call map_kernel_di
	lxi h,nmimsg
	jmp traphl

illegalmsg:
	.asciz '[illegal]'
nmimsg:
	.asciz '[NMI]'

!
!	Interrupts are similar to Z80 but we have a lot less state
!	to store, and rather trickier juggling to get signals nice
!
.define interrupt_handler

interrupt_handler:
	push psw
	INT_ENTER
	push b
	push d
	push h
	call plt_interrupt_all
	! Switch stacks
	lxi h,0
	dad sp
	shld istack_switched_sp
	lxi sp,istack_top

	!
	! Map the kernel
	!
	lda 0
	call map_save_kernel
	cpi 0xC3
	cnz null_pointer_trap
	!
	! Set up state and enter kernel
	!
	mvi a,1
	sta U_DATA__U_ININTERRUPT
	sta _int_disabled
	!
	!	What we avoid in register saves over Z80 we make up for in
	!	runtime stuff
	!
	lhld .retadr
	push h
	lhld .bcreg
	push h
	lhld .tmp1
	push h
	lhld .areg
	push h
	call _plt_interrupt
	pop h
	mov a,l
	sta .areg	! FIXME: add a pad byte to .areg instead
	pop h
	shld .tmp1
	pop h
	shld .bcreg
	pop h
	shld .retadr
	!
	! Do we need to task switch ?
	!
	lda _need_resched
	ora a
	jnz preemption
	!
	! Switch stacks back
	!
	call map_restore
	lhld istack_switched_sp
	sphl
intout:
	xra a
	sta U_DATA__U_ININTERRUPT
	lda U_DATA__U_INSYS
	ora a
	jnz interrupt_pop
	call deliver_signals
	!
	! Restore registers and done
	!
interrupt_pop:
	xra a
	sta _int_disabled
	pop h
	pop d
	pop b
	INT_EXIT
	pop psw
	ei
	ret

null_pointer_trap:
	mvi a,0xc3
	sta 0
	lxi h,11
trap_signal:
	push h
	lhld U_DATA__U_PTAB
	push h
	call _ssig
	pop h
	pop h
	ret

!
!	Now the scary stuff - preempting
!	
preemption:
	xra a
	sta _need_resched
	!
	!	Save our original stack in syscall_s
	!	Move to our kernel stack (free because we don't preempt
	!	in kernel
	!
	lhld istack_switched_sp
	shld U_DATA__U_SYSCALL_SP
	lxi sp,kstack_top
	!
	!	Mark ourselves as in a system call
	!
	mvi a,1
	sta U_DATA__U_INSYS
	call _chksigs
	lhld U_DATA__U_PTAB
	mvi a,P_RUNNING
	cmp m
	jnz not_running
	mvi m,P_READY
	!
	!	Set the flag so we are punished for using all our time
	!
	inx h
	mvi a,PFL_BATCH
	ora m
	mov m,a
not_running:
	!
	!	We will disappear into this and reappear somewhere else. In
	!	time we will reappear here
	!
	call _plt_switchout
	!
	!	We are back in the land of the living so no longer in
	!	syscall or interrupt state
	!
	xra a
	sta U_DATA__U_ININTERRUPT
	sta U_DATA__U_INSYS
	!
	!	Get our mapping back
	!
	call map_process_always_di
	!
	!	And our stack
	!
	lhld U_DATA__U_SYSCALL_SP
	sphl
	lda U_DATA__U_CURSIG
	ora a
	cnz deliver_signals_2
	jmp interrupt_pop

!
!	Debug code
!
.define outstring

outstring:
	mov a,m
	ora a
	rz
	call outchar
	inx h
	jmp outstring

.define outstringhex

outstringhex:
	mov a,m
	ora a
	rz
	call outcharhex
	mvi a,0x20
	call outchar
	inx h
	jmp outstringhex

.define outnewline

outnewline:
	mvi a,0x0d
	call outchar
	mvi a,0x0a
	jmp outchar

.define outhl

outhl:
	push psw
	mov a,h
	call outcharhex
	mov a,l
	call outcharhex
	pop psw
	ret

.define outde

outde:
	push psw
	mov a,d
	call outcharhex
	mov a,e
	call outcharhex
	pop psw
	ret

.define outbc

outbc:
	push psw
	mov a,b
	call outcharhex
	mov a,c
	call outcharhex
	pop psw
	ret

.define outcharhex

outcharhex:
	push b
	push psw
	mov c,a
	rar
	rar
	rar
	rar
	call outnibble
	mov a,c
	call outnibble
	pop psw
	pop b
	ret

outnibble:
	ani 0x0f
	cpi 10
	jc numeral
	adi 7
numeral:
	adi 0x30		! '0'
	jmp outchar


.define ___hard_ei

___hard_ei:
	xra a
	sta _int_disabled
	ei
	ret

.define ___hard_di

___hard_di:
	lxi h, _int_disabled
	di
	mov a,m
	mvi m,1
	mov e,a
	ret

.define ___hard_irqrestore

___hard_irqrestore:
	pop d
	pop h
	push h
	push d
	di
	mov a,l
	sta _int_disabled
	ora a
	rnz
	ei
	ret

!
!	Identify 8080 variants. We don't worry about Z80 variants. The 8080
!	kernel doesn't work on a Z80 so we don't care which one we have.
!
.define _cpu_detect

_cpu_detect:
	! Ok start with the flags
	mvi a,255
	inr a
	push psw
	pop h
	mov a,l
	ani 0x82
	cpi 0x80
	jz lr35902
	ora a
	jnz is808x
	lxi d,0		! Z80: we don't care which kind. It's simply not allowed
	ret
lr35902:
	lxi d,0		! also not allowed
	ret
is808x:
	xra a
	rim		! no-op on 8080
	ora a
	jnz is8085	! it changed must be an 8085
	!
	!	But it could really be 0
	!
	inr a
	rim
	ora a
	jz is8085
	!
	!	TODO: check for KP580M1
	!
	lxi d,8080
	!
	!	But wait it might be a 9080
	!
	mvi a,255
	ani 255
	push psw
	pop h
	mov a,l
	ani 0x10	! half carry is zero on AMD
	rnz
	mvi d,0x90
	ret
is8085:
	lxi d,0x8085
	ret


!
!	We need to worry about bits of this in interrupt save and restore
!

.define .trapproc, .retadr,.bcreg,.areg,.tmp1

.trapproc: .data2 0
.retadr: .data2 0
.bcreg: .data2 0
.areg: .data1 0
.tmp1: .data2 0

!
!	Errors from the runtime
!
.define eunimpl,eoddz,ecase,eidivz

eunimpl:
	lxi h,unimp
	jmp kerboom
eoddz:
	lxi h,ddz
	jmp kerboom
ecase:
	lxi h,case
	jmp kerboom
eidivz:
	lxi h,divz
kerboom:
	call outstring
	call _plt_monitor

unimp:	.asciz 'rt:unimp'
ddz:	.asciz 'rt:ddz'
case:	.asciz 'rt:case'
divz:	.asciz 'rt:div0'

.define _set_cpu_type

!
!	We use 08 to check what happens. On an 8080 it's a nop on an 8085
!	it does 16bit subtract, and on a Z80 its ex af,af' so uninteresting and
!	safe.
!
_set_cpu_type:
	push b
	lxi d,0
	lxi b,1
	dsub
	mov a,l
	ora a
	jz is8080		! 8080 or Z80
	mvi a,1
	sta _sys_cpu_feat	! 8085
is8080:
	pop b
	ret

!
!	CPU setup and properties. We are expecting an 8080, we might be
!	an 8085 or Z80 but we can't support the Z80 feature set as we don't
!	save IX, IY and alt registers. Maybe one day we will make the kernel
!	self patch to do both but given the performance gains it seems not
!	worth supporting Z80 booting with 8080 kernel.
!
.define _sys_cpu
.define _sys_cpu_feat
.define _sys_stubs

