#
;
;	VM low level code
;
;	Much the same as the Z80 code except we don't provide in and out
;	helpers because it's impossible to make them re-entrant for any port
;
#include "kernel-vm.def"

;
;	CPU setup and properties. As we are hardcoded for 8085 this isn't
;	too hard
;
		.data

.export _sys_cpu
.export _sys_cpu_feat
.export _sys_stubs

_sys_cpu:
	.byte 12	; Bytecode
_sys_cpu_feat:
	.byte 0		; No features

_sys_stubs:
	jump	unix_syscall_entry
	ds 13

.export delsig_2
.export delsig

		.common

delsig:
	z80call __bytecode
delsig_4:
	const U_DATA__U_CURSIG
	derefuc
	jtrue	delsig_3
	exit
	
delsig_2:
	z80call	__bytecode
delsig_3:
	push
	push
	plus
	push
	const U_DATA__U_SIGVEC
	plus
	push		; stack the address
	deref
	swap		; offset is now in A
	push
	constuc 0
	assign		; clear the signal vector

	; TOS is now the vector

	; Build the return frame
	const U_DATA__U_CURSIG
	derefuc
	swap
	push		; Stack is now vector, signum

	cpush 0
	const U_DATA__U_CURSIG
	assignc

	;	TOS is still the vector
	;
	pop
	jfalse	signal_return	; raced

	push
	;	Off we go TOS is vector
	constuc	1
	call irq
	pop
	callfunc

signal_return:
	constuc	0
	call irq
	getsp
	push
	const U_DATA__U_SYSCALL_SP
	assign
	const kstack_top
	setsp
	cpush 1
	const _int_disabled
	assignc
	call map_kernel_di
	call _chksigs
	call map_proc_always_di
	const U_DATA__U_SYSCALL_SP
	deref
	setsp
	jump delsig_4

.export unix_syscall_entry

unix_syscall_entry:
	z80call __bytecode

	push
	const U_DATA__U_CALLNO
	assignc

	loadl8	4
	deref
	push
	const U_DATA__U_ARGN
	assign
	loadl8	6
	deref
	push
	const U_DATA__U_ARGN+2
	assign
	loadl8	8
	deref
	push
	const U_DATA__U_ARGN+4
	assign
	loadl8	10
	deref
	push
	const U_DATA__U_ARGN+6
	assign

	constuc	1
	call irq

	; We are now in kernel space
	cpush 1
	const U_DATA__U_INSYS
	assignc
	
	; Switch stacks
	getsp
	push
	const U_DATA__U_SYSCALL_SP
	assign
	const kstack_top
	setsp
	;
	; Now map the kernel and call it
	;
	call map_kernel_di
	constuc 0
	call irq
	call _unix_syscall
	;
	; Remember fork and execve don't necessarily return this way and fork
	; can do it twice
	;
	constuc 1
	call irq
	call map_proc_always
	cpush 0
	const U_DATA__U_INSYS
	assign
	; Switch stack back
	const U_DATA__U_SYSCALL_SP
	deref
	setsp

	const U_DATA__U_RETVAL
	deref
	push
	const U_DATA__U_ERROR
	deref
	push
	;
	; Signal check
	;
	const U_DATA__U_CURSIG
	derefuc
	jtrue via_signal
unix_return:
	; TOS is error, retval
	constuc 0
	call irq
	constuc 16
	shl		; error << 16
	or		; | data
	exit
via_signal:
	; error state is on kernel stack so we can recover it
	; after the signal return correctly for the syscall ret
	;
	; And into the signal delivery path
	;
	call delsig_2
	; Return data is on stack as we need it
	jump unix_return

;
;	Called when execve() completes to transition to the user, as we
;	don't return from execve() via the syscall path
;
;
.export _doexec

_doexec:
	z80call __bytecode
	constuc 1
	call irq
	call map_proc_always
	const U_DATA__U_ISP
	deref
	setsp
	cpush 0
	const U_DATA__U_INSYS
	assignc
	call irq
	pop		; return address (unused)
	pop		; execute address
	callfunc

;
;	Interrupt (very primitive for nwo)
;
.export interrupt_handler

interrupt_handler:
	push
	INT_ENTER
	call plt_interrupt_all
	getsp
	push
	const istack_switched_sp
	assign
	const istack_top
	setsp

	;
	; Map the kernel
	;
	call map_save_kernel
	;
	; Set up state and enter kernel
	;
	cpush 1
	const U_DATA__U_ININTERRUPT
	assignc
	push
	const _int_disabled
	assignc

	call _plt_interrupt

	;
	; Do we need to task switch ?
	;
	const _need_resched
	derefuc
	jtrue preemption
	;
	; Switch stacks back
	;
	call map_restore
	const istack_switched_sp
	deref
	setsp
intout:
	cpush 0
	const U_DATA__U_ININTERRUPT
	assign

	const U_DATA__U_INSYS
	derefuc
	jtrue interrupt_pop
	call delsig
	;
	; Restore registers and done
	;
interrupt_pop:
	cpush 0
	const _int_disabled
	assignc
	constuc 0
	call irq
	pop
	exit

;
;	Now the scary stuff - preempting
;	
preemption:
	cpush 0
	const _need_resched
	assign
	;
	;	Save our original stack in syscall_s
	;	Move to our kernel stack (free because we don't preempt
	;	in kernel
	;
	const istack_switched_sp
	deref
	push
	const U_DATA__U_SYSCALL_SP
	assign
	const kstack_top
	setsp

	;
	;	Mark ourselves as in a system call
	;
	cpush 1
	const U_DATA__U_INSYS
	assignc
	call _chksigs
	const U_DATA__U_PTAB
	deref
	push
	derefuc
	push
	constuc P_RUNNING
	cceq
	jfalse not_running
	swap
	dup
	constuc P_READY
	assignc
	constuc 1
	plus		; ptr + 1 (flags)
	dup
	derefuc
	push
	const PFL_BATCH
	or
	assign
	push		; just push randoness to keep stack balanced with
			; other path
not_running:
	pop
	;
	;	We will disappear into this and reappear somewhere else. In
	;	time we will reappear here
	;
	call _plt_switchout
	;
	;	We are back in the land of the living so no longer in
	;	syscall or interrupt state
	;
	cpush	0
	dup
	const	U_DATA__U_ININTERRUPT
	assign
	const	U_DATA__U_INSYS
	assign
	;
	;	Get our mapping back
	;
	call map_proc_always_di
	;
	;	And our stack
	;
	const U_DATA__U_SYSCALL_SP
	deref
	setsp
	const U_DATA__U_CURSIG
	derefuc
	jfalse interrupt_pop
	call delsig_2
	jump interrupt_pop

;
;	Debug code
;
.export outstring

outstring:
	z80call __bytecode
outstringl:
	push
	dup
	derefuc
	jfalse outdone
	call outchar
	constuc 1
	plus
	jump outstringl
outdone:
	exit

.export outstringhex

outstringhex:
	z80call __bytecode
outstringhexl:
	push
	dup
	derefuc
	jfalse outdone
	call outcharhex
	constuc 0x20
	call outchar
	constuc 1
	plus
	jump outstringhexl

.export outnewline

outnewline:
	const 0x0d
	call outchar
	const 0x0a
	call outchar
	exit

.export outcharhex

outcharhex:
	z80call __bytecode
	push
	z80call __bytecode
	push
	push
	constuc 4
	shru
	call outnibble
	const 15
	band
	call outnibble
	pop
	exit

outnibble:
	z80call __bytecode
	push
	push
	const 9
	ccgt
	jfalse numeral
	constuc 7
	plus
	push
numeral:
	const 0x30		; '0'
	plus
	call outchar
	exit


.export ___hard_ei

___hard_ei:
	z80call __bytecode
	cpush 0
	const _int_disabled
	assign
	call irq
	exit

.export ___hard_di

___hard_di:
	z80call __bytecode
	const _int_disabled
	dup
	deref
	swap
	constuc 1
	swap
	assign
	pop
	exit

.export ___hard_irqrestore

___hard_irqrestore:
	z80call __bytecode
	loadl 2
	call irq
	push
	const _int_disabled
	assignc
	call irq
	exit

;
;	Identify 8080 variants. We don't worry about Z80 variants. The 8085
;	kernel doesn't work on a Z80 so we don't care which one we have.
;
.export _cpu_detect

_cpu_detect:
	z80call __bytecode
	constuc 0
	exit

.export _set_cpu_type

_set_cpu_type:
	z80call __bytecode
	exit


