#include "../kernel-vm.def"
	.common

.export _plt_switchout

;
;	The ABI requires we preserve BC
;

;
;	Switch a process out and run the next one
;
_plt_switchout:
	z80call __bytecode
	constuc	0
	push
	const	U_DATA__U_SP
	push
	getsp
	assign
	call 	ap_proc_always_di
	;
	;	Save the udata into process state
	;
	;	FIXME: can we skip this if the process is defunct or would
	;	it even make sense to defer it (with magic in swapout) ?
	;
	cpush	512
	cpush	_udata
	cpush	U_DATA__STASH
	call	_memcpy
	spmod8	6
	call	_getproc		; Who goes next
	push
	call	_switchin		; Run it
	call	_plt_monitor		; Should never hit this
;
;	Switch a process back in
;
.export _switchin

;
;	Switch to a given process
;
_switchin:
	z80call __bytecode
	constuc	1
	call irq
	loadl8	2
	deref
	push
	push
	const	procptr
	assign
	pop

	call	map_kernel_di

	;
	;	Is it swapped (page 0)
	;
	push
	derefo	P_TAB__P_PAGE_OFFSET
	jfalse	not_swapped

	; Swap

	const	swapstack
	setsp

	;
	;	Swapping time. Interrupts back on
	;	FIXME: not clear this is safe for re-entry on swap when
	;	in IRQ path
	;
;;	constuc	0
;;	call irq
;;	push
;;	const	_int_disabled
;;	assignb

	const	procptr
	deref
	push
	call	_swapper
	pop
	;
	;	It should now be back in memory
	;
	constuc	1
	push
	const	_int_disabled
	assignc
	constuc	1
	call irq
	;
	;	Check if we need to recover the udata (we were last to run)
	;
not_swapped:
	; Get onto swap stack anyway, with the VM this is the eaisest way
	; to do the switch
	const	swapstack
	setsp
	const	procptr
	deref
	push
	push
	const	U_DATA__U_PTAB
	deref
	cceq
	jtrue	skip_copyback
	;
	;	Recover the udata
	;
copyback:
	pop
	push
	derefouc	P_TAB__P_PAGE
	call	map_proc_a

	cpush	512
	cpush	U_DATA_U_STASH
	cpush	_udata

	call	_memcpy
	spmod8	6

	const	U_DATA__U_SP
	deref
	setsp

	call	map_kernel
	;
	;	Did we find the right process ?
	;
	const	procptr
	deref
	push
	const	U_DATA__U_PTAB
	deref
	cceq
	jfalse	switchinfail

skip_copyback:

	;	Mark us as running, clear our preemption counter
	;	and set the interrupt flags
	;
	cpush	P_RUNNING
	const	U_DATA__U_PTAB
	deref
	assignc
	const	U_DATA__U_PTAB
	deref
	derefo	P_TAB__P_PAGE_OFFSET
	push
	const	U_DATA__U_PAGE
	assign
	cpush	0
	const	_runticks
	assign
	const	U_DATA__U_SP
	deref
	setsp

	const	U_DATA__U_ININTERRUPT
	derefuc
	push
	const	_int_disabled
	assignc
	call irq
	exit

switchinfail:
	call	outa
	const	badswitchmsg
	call	outstring
	call	_plt_monitor

badswitchmsg:
	.ascii 'badsw'
	.byte 0


procptr:
	.word 0

.export _dofork
;
;	The heart of fork
;
_dofork:
	z80call __bytecode
	loadl8	2
	push
	const	procptr
	assign

	derefo	P_TAB__P_PID_OFFSET
	deref
	push

	getsp
	push
	const	U_DATA__U_SP
	assign
	;
	; We are now in a safe state to work
	;
	const	procptr
	derefo	P_TAB__P_PAGE_OFFSET

	push
	call	setdbank
	pop

	cpush	49152
	cpush	0
	dup
	call	map_proc_always
	call	_bankcopy
	spmod8	6

	;
	;	Copy the parent udata and stack into the parent stash
	;	The live udata becomes that of the child
	;
	cpush	512
	cpush	U_DATA_STASH
	cpush	_udata
	call	_memcpy
	spmod8	6

	call	map_kernel
	pop			; Saved PID goes away 

	cpush	_udata
	const	procptr
	deref
	push
	call	_makeproc
	spmod8	4
	;
	;	Timer ticks
	;
	cpush	0
	const	_runticks
	assign
	;
	;	Return A = 0 (child)
	;
	exit

.export _swapstack

	ds 256			; Do we really need 256 ?
_swapstack:

.export _need_resched

_need_resched:
	.byte 0
