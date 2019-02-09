!
!	For a purely bank based architecture this code is common and can be
!	used by most platforms
!
!	The caller needs to provide the standard map routines along with
!	map_kernel_a which maps in the kernel bank in a. This code assumes
!	that the bank can be encoded in 8bits.
!

.sect .common

.define _platform_switchout

!
!	The ABI requires we preserve BC
!

!
!	Switch a process out and run the next one
!
_platform_switchout:
	lxi h,0
	push h			! Save a 0 argment
	push b
	dad sp
	shld U_DATA__U_SP	! Save the sp for a switch in
	call map_process_always_di
	!
	!	Save the udata into process state
	!
	lxi h,U_DATA
	lxi d,U_DATA_STASH
	call copy512		! No ldir 8(
	! Back to kernel
	call map_kernel_di
	call _getproc		! Who goes next
	push h
	call _switchin		! Run it
	! Should never hit this
	call _platform_monitor

!
!	Switch a process back in
!
.define _switchin

!
!	Switch to a given process
!
_switchin:
	di
	pop b
	pop d			! DE is now our process to run
	push d
	push b

	call map_kernel_di

	!
	!	Is it swapped (page 0)
	!
	lxi h,P_TAB__P_PAGE_OFFSET
	dad d

	lxi sp,_swapstack
	mov a,m
	ora a
	jnz not_swapped
	!
	!	Swapping time. Interrupts back on
	!
	ei
	xra a
	sta _int_disabled
	!
	!	Can we rely on the compiler not mashing the stacked var ?
	!
	push h
	push d
	call _swapper
	pop d
	pop h
	!
	!	It should now be back in memory
	!
	mvi a,1
	sta _int_disabled
	di
	mov a,m
	!
	!	Check if we need to recover the udata (we were last to run)
	!
not_swapped:
	mov c,a
	lhld U_DATA__U_PTAB
	mov a,h
	cmp d
	jnz copyback
	mov a,l
	cmp e
	jz skip_copyback
	!
	!	Recover the udata
	!
copyback:
	mov a,c
	call map_process_a
	
	lxi h,U_DATA_STASH
	lxi d,U_DATA
	call copy512

	lhld U_DATA__U_SP	! A valid stack to map on
	sphl

	call map_kernel
	!
	!	Did we find the right process ?
	!
	lhld U_DATA__U_PTAB
	mov a,h
	cmp d
	jnz switchinfail
	mov a,l
	cmp e
	jnz switchinfail

skip_copyback:
	!
	!	Mark us as running, clear our preemption counter
	!	and set the interrupt flags
	!
	lhld U_DATA__U_PTAB
	mvi a,P_RUNNING
	mov m,a
	lxi d,P_TAB__P_PAGE_OFFSET
	dad d
	mov a,m
	sta U_DATA__U_PTAB
	lxi h,0
	shld _runticks
	lhld U_DATA__U_SP
	sphl
	!
	!	Recover our parent frame pointer and return code
	!
	pop b
	pop h
	lda U_DATA__U_ININTERRUPT
	sta _int_disabled
	ora a
	rnz
	ei
	ret

switchinfail:
	call outhl
	lxi h,badswitchmsg
	call outstring
	call _platform_monitor

badswitchmsg:
	.asciz 'badsw'


fork_proc_ptr:
	.data2 0

.define _dofork
!
!	The heart of fork
!
_dofork:
	pop d
	pop h		! new process
	push h
	push d

	shld fork_proc_ptr

	lxi d,P_TAB__P_PID_OFFSET
	dad d
	mov a,m
	inx h
	mov h,m
	mov l,a
	!
	! We don't have any state to save but the pid and framepointer 
	!
	push b
	push h
	lxi h,0
	dad sp
	shld U_DATA__U_SP
	!
	! We are now in a safe state to work
	!
	lhld fork_proc_ptr
	lxi d,P_TAB__P_PAGE_OFFSET
	dad d
	mov c,m
	lda U_DATA__U_PAGE

	call bankfork

	call map_process_always
	!
	!	Clone the parent udata and stack into the child stash
	!
	lxi h,U_DATA
	lxi d,U_DATA_STASH
	call copy512

	call map_kernel

	pop h		! Get rid of saved pid

	!
	!	Manufacture the child udata state
	!
	lxi h,_udata
	push h
	lhld fork_proc_ptr
	push h
	call _makeproc
	pop b
	pop b
	!
	!	Timer ticks
	!
	lxi h,0
	shld _runticks
	!
	!	Frame pointer
	!
	pop b
	ret

.define bouncebuffer
.define _swapstack

bouncebuffer:
	.space 256		! Do we really need 256 ?
_swapstack:

.define _need_resched

_need_resched:
	.data1 0
