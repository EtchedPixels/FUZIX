; We are in DI so we can poke these directly but must not invoke
; any code outside of common

.macro	switch
	ld bc,#0x7ffd
	or #BANK_BITS
	out (c),a
.endm

        .module tricks

        .globl _ptab_alloc
        .globl _makeproc
	.globl _udata
        .globl _getproc
        .globl _plt_monitor
        .globl _plt_switchout
        .globl _switchin
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
	.globl current_map
	.globl _ptab
	.globl _swapper
	.globl _int_disabled
	.globl switch_bank
	.globl _inswap

	.globl _switchedwb
	.globl _switchedbank

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .include "kernel.def"
        .include "../kernel-z80.def"

        .area _COMMONMEM

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_plt_switchout:
        di
        ; save machine state

        ld hl,	#0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push	hl ; return code
        push	ix
        push	iy

	ld	a,(current_map)
	push	af

        ld	(_udata + U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

	;
	; We are now running on the sleeping process stack. The switchin
	; will simply go back to the saved SP above and discard anything
	; here
	;

	; Stash the uarea back into process memory
	ld a, (_udata + U_DATA__U_PAGE)

	; Macro as this depends on the clone
	switch

	; This includes the stacks, so be careful on restore
	ld hl,	#_udata
	ld de,	#U_DATA_STASH
	ld bc,	#U_DATA__TOTALSIZE
	ldir

	ld a,	(current_map)

	switch

        ; find another process to run (may select this one again)
	push af
        call	 _getproc
	pop af		; we can't optimise this as the linker
			; is entitled to patch the 5 bytes here into a
			; banked call
        push hl
	push af
        call	_switchin

        ; we should never get here
        call	_plt_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0

_switchin:
        di
	pop	hl	; far padding
        pop	bc  ; return address
        pop	de  ; new process pointer
;
;	FIXME: do we actually *need* to restore the stack !
;
        push	de ; restore stack
        push	bc ; restore stack
	push	hl ; far padding

        ld	hl, #P_TAB__P_PAGE_OFFSET
 	add	hl, de	; process ptr

	;	HL now points to the pages for this process, DE to the
	;	process
	;
	;	Get ourselves a valid private stack ASAP. We are going to
	;	copy udata around and our main stacks are in udata
	;
	ld	sp, #_swapstack

	;
	;	Do we need to swap in the new process ?
	;

        ld	a, (hl)	; 0 swapped, not zero is the bank for 0x8000
	or	a
	jp	nz, not_swapped

	; Swap the process in (this may swap something else out first)
	; The second pushes are C function arguments. SDCC can trample
	; these

	ld	a,(_switchedwb)
	or	a
	jr	z, nowback

	;
	; We do lazy writeback of the 16K block.
	; Q: can it ever be the process being swapped out and optimize
	; from the swap path. Not in any normal situation I think.
	;
	ld	a,(_switchedbank)

	exx

	ld	bc,#0x013B
	out	(c),a

	ld	hl,#0x6000
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	ld	a,#0xC5		; C4 C5 C0
	ld	bc,#0x013B
	out	(c),a

	exx

	xor	a
	ld	(_switchedwb),a	; c
	;
	; Our pages are now consistent
	;
nowback:
	; Turn this on once we have it all sorted and debugged
	xor	a
;	ld (_int_disabled),a
	inc	a
	ld	(_inswap),a
;NOTYET	ei
	push	hl		; Save
	push	de
	push	hl		; Arguments
	push	de
	push	af
	call	_swapper
	pop	af
	pop	af
	pop	af
	pop	de		; Restore
	pop	hl
	;
	; We have been loaded into 6000-FFFF directly
	;
	ld	a,#1
	ld	(_int_disabled),a
	dec	a
	di
	ld	(_inswap),a
	; Remember the swapper set up the 6000-BFFF range for us
	; and it is not clean so must be written back in future
	inc	hl
	inc	hl			; point to PAGE_2
	ld	a,(hl)
	ld	(_switchedbank),a
	ld	a,#1
	ld	(_switchedwb),a
	dec	hl
	dec	hl
	;
	; We differ from most here because if we swapped in we put the page
	; at 8000-BFFF in the right place, if not we need to ldir it
	;
	jp no_relocate

	;
	; Mark the writeback as dirty
	;
no_relocate_w:
	dec hl
	dec hl			; point to high page
	jp no_relocate
	;
	; The non swap case. There are three basic variants
	; 1. Someone else ran before us and we must write them back
	; and copy ourselves in
	; 2. Someone else ran before us and died. We only need to copy our
	; own pages in
	; 3. We ran, there were multiple idle candidates, we slept and we
	; happen to be the first back. In which case we need to do nothing
	;
not_swapped:
	;
	; This means 6000-BFFF is wrong. However it might be wrong for a
	; reason we don't care about (previous owned popped their clogs)
	;
	ld	a,(_switchedbank)	; current low page
	inc	hl
	inc	hl		; new low page

	cp	(hl)		; is the page mapped low ours ?
	jr	z,no_relocate_w

	or	a		; is it dead ?
	jr	z, no_copyback

	;
	; This has already been written back
	;
	ld	a,(_switchedwb)
	or	a
	jr	z, no_copyback


	ld	a,(_switchedbank)
	;
	; Copy 6 x 4K into the spectranet RAM
	;
	exx

	ld	bc,#0x013B
	out	(c),a

	ld	hl,#0x6000
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	ld	a,#0xC5		; C4 C5 C0
	ld	bc,#0x013B
	out	(c),a
	exx

no_copyback:
	ld	a,(hl)		; p->p_page2
	ld	(_switchedbank),a	; What will be present in the bank

	;
	; Copy 6 x 4K from the spectranet RAM
	;
	exx

	ld	bc,#0x013B
	out	(c),a

	ld	hl,#0x2000
	ld	de,#0x6000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	hl,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	hl,#0x2000
	ld	bc,#0x013B
	out	(c),a
	ld	bc,#0x1000
	ldir

	inc	a
	ld	hl,#0x2000
	ld	bc,#0x013B
	out	(c),a
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	hl,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	hl,#0x2000
	ld	bc,#0x1000
	ldir

	ld	a,#0xC5		; C4 C5 C0
	ld	bc,#0x013B
	out	(c),a

	exx
	dec	hl
	dec	hl
	;
	; Entered with HL = &page
	;
no_relocate:
	ld	a,#1
	ld	(_switchedwb),a		; Writeback will be required

	ld	a,(hl)	; p->p_page

	ld	hl,(_udata + U_DATA__U_PTAB)
	or	a
	sbc	hl,de			; valid udata ?
	jr z,	skip_copyback

	;	Pages please
	switch

	;	Copy the stash from the user page back down into common
	;	The alternate registers are free - we use them for the
	;	block copies
	;
	exx
	ld	hl, #U_DATA_STASH
	ld	de, #_udata
	ld	bc, #U_DATA__TOTALSIZE
	ldir
	exx


skip_copyback:
	;
	;	 Remap the kernel proper
	;

	ld	a, (current_map)
	switch

        ; check u_data->u_ptab matches what we wanted
        ld	hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or	a                    ; clear carry flag
        sbc	hl, de              ; subtract, result will be zero if DE == HL
        jr	nz, switchinfail

	; wants optimising up a bit
	ld	ix, (_udata + U_DATA__U_PTAB)
        ; next_process->p_status = P_RUNNING
        ld	P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING

	; Fix any moved page pointers
	; Just do one byte of each as that is all we use on this platform
	ld	a, P_TAB__P_PAGE_OFFSET(ix)
	ld	(_udata + U_DATA__U_PAGE), a
	ld	a, P_TAB__P_PAGE2_OFFSET(ix)
	ld	(_udata + U_DATA__U_PAGE2), a
        ; runticks = 0
        ld	hl, #0
        ld	(_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld	sp, (_udata + U_DATA__U_SP)

	;
	; We can now use the stack again
	;

	pop	af
        pop	iy
        pop	ix
        pop	hl ; return code

	; Make sure we have the right kernel bank to return to
	call	switch_bank

        ; enable interrupts, if the ISR isn't already running
        ld	a, (_udata + U_DATA__U_ININTERRUPT)
	ld	(_int_disabled),a
        or	a
        ret	nz ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
	call	outhl
        ld	hl, #badswitchmsg
        call	outstring
	; something went wrong and we didn't switch in what we asked for
        jp	_plt_monitor

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        di ; should already be the case ... belt and braces.

	pop	bc
        pop	de  ; return address
        pop	hl  ; new process p_tab*
        push	hl
        push	de
	push	bc

        ld	(fork_proc_ptr), hl

        ; prepare return value in parent process -- HL = p->p_pid;
        ld	de, #P_TAB__P_PID_OFFSET
        add	hl, de
        ld	a, (hl)
        inc	hl
        ld	h, (hl)
        ld	l, a

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        push	hl ; HL still has p->p_pid from above, the return value in the parent
        push	ix
        push	iy

	ld	a,(current_map)
	push	af

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
        ld	(_udata + U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	; Need to write a new 16K bank copy here, then copy the live uarea
	; into the stash of the new process

        ; --------- copy process ---------

        ld	hl, (fork_proc_ptr)
        ld	de, #P_TAB__P_PAGE_OFFSET	;	bank number for high page
        add	hl, de
        ; load p_page for the high page
        ld	c, (hl)
	push	hl

	ld	hl, (_udata + U_DATA__U_PAGE)
	ld	a, l

	;
	; Copy the high page via the bounce buffer
	;

	call	bankfork		;	do the bank to bank copy

	; FIXME: if we support small apps at C000-FBFF we need to tweak this
	; Now copy the 0x6000-0xBFFF area directly back into the parent
	; the child inherits the existing 6000-BFFF and will in turn write
	; it back when it gets switched out

	ld	a,(_udata + U_DATA__U_PAGE2)	;	low page of parent

	ld	bc,#0x013B
	out	(c),a

	ld	hl,#0x6000
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	inc	a
	ld	bc,#0x013B
	out	(c),a
	ld	de,#0x2000
	ld	bc,#0x1000
	ldir

	ld	a,#0xC5		; C4 C5 C0
	ld	bc,#0x013B
	out	(c),a


	; We have one weirdness left. When we copied these up to the parent
	; we made the child the owner

	pop	hl
	inc	hl
	inc	hl			;	Child low page pointer
	ld	a,(hl)
	ld	(_switchedbank),a	;	child bank (p->p_page2)
	ld	a,#1
	ld	(_switchedwb),a

	; Copy done

	ld a, (_udata + U_DATA__U_PAGE)		; parent memory high

	switch			; Switch context to parent in 0xC000+

	; We are going to copy the uarea into the parents uarea stash
	; we must not touch the parent uarea after this point, any
	; changes only affect the child
	ld hl, #_udata		; copy the udata from common into the
	ld de, #U_DATA_STASH	; target process
	ld bc, #U_DATA__TOTALSIZE
	ldir

	;
	; And back into the kernel
	;
	ld a, (current_map)
	switch
        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.

        pop	bc
        pop	iy
        pop	ix
	pop	bc

        ; Make a new process table entry, etc.
	ld	hl, #_udata
	push	hl
        ld	hl, (fork_proc_ptr)
        push	hl
	push	af
        call	_makeproc
	pop	af
        pop	bc 
	pop	bc

        ; runticks = 0;
        ld	hl, #0
        ld	(_runticks), hl
        ; in the child process, fork() returns zero.
	;
	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        ret

;
;	This is related so we will keep it here. Copy the process memory
;	for a fork. a is the page base of the parent, c of the child
; 	(this API will be insufficient once we have chmem and proper use of
; 	banks - as well as needing to support fork to disk)
;
;	Assumption - fits into a fixed number of whole 256 byte blocks
;
;
;	Note: this needs reviewing. We now have a lot more program memory
;	we can use with a lazy copying model
;
bankfork:
	ld	b, #0x3E	; 64 x 256 minus 2 sets for the uarea stash/irqs
	ld	hl, #0xC000	; base of memory to fork (vectors included)
bankfork_1:
	push	bc		; Save our counter and also child offset
	push	hl

	switch

	ld	de, #bouncebuffer
	ld	bc, #256
	ldir			; copy into the bounce buffer
	pop	de		; recover source of copy to bounce
				; as destination in new bank
	pop	bc		; recover child port number
	push	bc
	ld	b, a		; save the parent bank id
	ld	a, c		; switch to the child
	push	bc		; save the bank pointers

	switch

	ld	 hl, #bouncebuffer
	ld	bc, #256
	ldir			; copy into the child
	pop	bc		; recover the bank pointers
	ex	de, hl		; destination is now source for next bank
	ld	a, b		; parent back is wanted in a
	pop	bc
	djnz	bankfork_1	; rinse, repeat
	ret
;
;	For the moment
;
	.area _COMMONDATA
bouncebuffer:
	.ds	256
;	We can keep a stack in common because we will complete our
;	use of it before we switch common block. In this case we have
;	a true common so it's even easier. We never use both at once
;	so share with bouncebuffer
_swapstack:
_switchedbank:
	.db	0
_switchedwb:
	.db	0

fork_proc_ptr:
	.dw	0 ; (C type is struct p_tab *) -- address of child process p_tab entry
