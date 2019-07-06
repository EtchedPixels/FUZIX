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
	push b
	lxi h,0
	push h			! Save a 0 argment
	lhld .retadr
	push h
	lhld .bcreg
	push h
	lhld .tmp1
	push h
	lhld .areg
	push h
	! Do we need to save block1->block3+2 (ie do we do 32bit * and / ???
	lxi h,0
	dad sp
	shld U_DATA__U_SP	! Save the sp for a switch in
	call map_process_always_di
	!
	!	Save the udata into process state
	!
	!	FIXME: can we skip this if the process is defunct or would
	!	it even make sense to defer it (with magic in swapout) ?
	!
	lxi h,U_DATA
	lxi d,U_DATA_STASH
	call copy512		! No ldir 8(
	! Back to kernel
	call map_kernel_di
	call _getproc		! Who goes next
	push d
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
	mov a,m			! A is now our bank
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
	! DE is the process to run, get the udata process into HL
	lhld U_DATA__U_PTAB
	! See if our udata is live - this is common, if we sleep and we are
	! the next to run for example.
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

	push d
	lxi h,U_DATA_STASH
	lxi d,U_DATA
	call copy512
	pop d

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
	ldhi P_TAB__P_PAGE_OFFSET
	ldax d
	sta U_DATA__U_PAGE
	lxi h,0
	shld _runticks
	lhld U_DATA__U_SP
	sphl
	!
	!	Recover our parent frame pointer and return code
	!
	!	probably we still need to save block1-block3 (12 bytes)
	!	used by .mli4/.dvi4. Would be good to make them do their
	!	own saves so the cost is paid in the right place
	!
	pop h
	mov a,l
	sta .areg	! FIXME: add a pad byte to .areg instead
	pop h
	shld .tmp1
	pop h
	shld .bcreg
	pop h
	shld .retadr
	pop d
	pop b
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
	ldsi 2
	lhlx

	shld fork_proc_ptr

	lxi d,P_TAB__P_PID_OFFSET
	dad d
	mov a,m
	inx h
	mov h,m
	mov l,a
	!
	! We don't have any state to save but the pid and framepointer 
	! and (alas) a pile of compiler non-reentrant crap
	!
	push b
	push h
	lhld .retadr
	push h
	lhld .bcreg
	push h
	lhld .tmp1
	push h
	lhld .areg
	push h
	lxi h,0
	dad sp
	shld U_DATA__U_SP
	!
	! We are now in a safe state to work
	!
	lhld fork_proc_ptr
	ldhi P_TAB__P_PAGE_OFFSET
	xchg
	mov c,m
	lda U_DATA__U_PAGE

	call bankfork

	call map_process_always
	!
	!	Copy the parent udata and stack into the parent stash
	!	The live udata becomes that of the child
	!
	lxi h,U_DATA
	lxi d,U_DATA_STASH
	call copy512

	call map_kernel

	pop h		! Get rid of saved pid
	pop h		! and C runtime state
	pop h
	pop h
	pop h

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
	xchg			! return 0 in DE for child
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
