#include "../kernel-8085.def"

	.common

	.setcpu 8085

.export _plt_switchout

;
;	The ABI requires we preserve BC
;

;
;	Switch a process out and run the next one
;
_plt_switchout:
	lxi h,0			; save a 0 return
	;
	; We don't have any state to save but the pid and BC
	; and (alas) a pile of compiler non-reentrant crap
	;
	push	b		; old BC
	push	h		; pid
	lhld	__tmp
	push	h
	lhld	__hireg
	push	h
	lhld	__tmp2
	push	h
	lhld	__tmp2+2
	push	h
	lhld	__tmp3
	push	h
	lhld	__tmp3+2
	push	h
	lhld	__retaddr
	push	h
	lxi h,0
	dad sp
	shld U_DATA__U_SP	; Save the sp for a switch in
	call _getproc		; Who goes next
	push h
	call _switchin		; Run it
	; Should never hit this
	call _plt_monitor

;
;	Switch a process back in
;
.export _switchin

;
;	Switch to a given process
;
_switchin:
	di
	pop b
	pop d			; DE is now our process to run
	push d
	push b

	call map_kernel_di

	;
	;	Is it swapped (page 0)
	;
	lxi h,P_TAB__P_PAGE_OFFSET
	dad d

	lxi sp,_swapstack
	mov a,m			; A is now our bank
	ora a
	jnz not_swapped
	;
	;	Swapping time. Interrupts back on
	;
	ei
	xra a
	sta _int_disabled
	;
	;	Can we rely on the compiler not mashing the stacked var ?
	;
	push h
	push d
	call _swapper
	pop d
	pop h
	;
	;	It should now be back in memory
	;
	mvi a,1
	sta _int_disabled
	di
	mov a,m
	;
	;	Check if we need to recover the udata (we were last to run)
	;
not_swapped:
	;	Needs care as we switch stack and upper bank here
	;	Must not touch stack or allow ints untiul SP is set
	ani 0x54
	ori 3
	sta curmap
	out 0xFF

	;	Mark us as running, clear our preemption counter
	;	and set the interrupt flags
	;
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
	;
	;	Recover our parent frame pointer and return code
	;
	pop	h
	shld	__retaddr
	pop	h
	shld	__tmp3+2
	pop	h
	shld	__tmp3
	pop	h
	shld	__tmp2+2
	pop	h
	shld	__tmp2
	pop	h
	shld	__hireg
	pop	h
	shld	__tmp
	pop h
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
	call _plt_monitor

badswitchmsg:
	.ascii 'badsw'
	.byte 0



.export _dofork
;
;	The heart of fork
;
_dofork:
	ldsi	2
	lhlx

	shld	fork_proc_ptr

	lxi	d,P_TAB__P_PID_OFFSET
	dad	d
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	;
	; We don't have any state to save but the pid and BC
	; and (alas) a pile of compiler non-reentrant crap
	;
	push	b		; old BC
	push	h		; pid
	lhld	__tmp
	push	h
	lhld	__hireg
	push	h
	lhld	__tmp2
	push	h
	lhld	__tmp2+2
	push	h
	lhld	__tmp3
	push	h
	lhld	__tmp3+2
	push	h
	lhld	__retaddr
	push	h
	lxi	h,0
	dad	sp
	shld	U_DATA__U_SP
	;
	; We are now in a safe state to work
	;
	lhld	fork_proc_ptr
	ldhi	P_TAB__P_PAGE_OFFSET
	xchg
	mov	c,m
	lda	U_DATA__U_PAGE

	; This bit gets exciting. We need to copy the low 56K but also the
	; udata in our per task common copy.

	call	bankfork

	; Returns in child mapping

	lxi	h,16	; Drop the stack frame
	dad	sp	; (it's still in the other copy)
	sphl

	;
	;	Manufacture the child udata state
	;

	lxi	h,_udata
	push	h
	lhld	fork_proc_ptr
	push	h
	call	_makeproc
	pop	d
	pop	d
	;
	;	Timer ticks
	;
	lxi	h,0
	shld	_runticks
	;
	;	Recover B and return HL = 0 (child)
	;
	pop	b
	ret

.export _swapstack

; This stack exists in each bank.
	.ds 256			; Do we really need 256 ?
_swapstack:

.export _need_resched

; TODO: can this live in common ?
_need_resched:
	.byte 0

.export bankfork

; Always called with interrupts off

bankfork:
	; A is source C is destination banking. The code we are patching
	; is split across banks so we must patch the right bank version
	; of each point

	mov b,a		; save parent bank
	lxi d,0		; working address
	lxi h,0xE200/8	; loop count
	; Could be optimized with sp tricks but sanity is probably better
	; as the 8085 can be clocked much faster

	out 0xFF
copy:
	push h
	lhlx
	mov a,c
	out 0xFF
	shlx
	inx d
	inx d
	mov a,b
	out 0xFF
	lhlx
	mov a,c
	out 0xFF
	shlx
	inx d
	inx d
	mov a,b
	out 0xFF
	lhlx
	mov a,c
	out 0xFF
	shlx
	inx d
	inx d
	mov a,b
	out 0xFF
	lhlx
	mov a,c
	out 0xFF
	shlx
	inx d
	inx d
	mov a,b
	out 0xFF
	pop h
	dcx h
	jnk copy
	; We are in the child map so just tidy up and return
	mov a,c
	ani 0x54		; child with kernel low
	ori 0x03
	out 0xFF
	sta curmap
	ret

	.data

fork_proc_ptr:
	.word 0
