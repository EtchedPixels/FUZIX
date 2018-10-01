; 2013-12-21 William R Sowerbutts

        .module tricks

        .globl _ptab_alloc
        .globl _newproc
        .globl _chksigs
        .globl _getproc
        .globl _platform_monitor
        .globl trap_illegal
        .globl _inint
        .globl _platform_switchout
        .globl _switchin
        .globl _doexec
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
        .globl dispatch_process_signal
	.globl _swapper
	.globl _cached_page
	.globl _flush_cache
	.globl _invalidate_cache

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .include "kernel.def"
        .include "../kernel.def"

        .area _COMMONMEM

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_platform_switchout:
        di
        ; save machine state

        ld hl, #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
        push ix
        push iy
        ld (U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

        ; set inint to false
        xor a
        ld (_inint), a

	; Stash the uarea back into process memory
	ld hl, (U_DATA__U_PAGE)
	ld a, l
	out (21), a
	cp h			; small or large process ?
	jr z, switchoutlow
	ld de, #U_STASH_HIGH
switchoutlow:
	ld hl, #U_DATA
	ld de, #U_STASH_LOW
	ld bc, #U_DATA__TOTALSIZE
	ldir
	xor a
	out (21), a

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _platform_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
            .db 13, 10, 0

_switchin:
        di
        pop bc  ; return address
        pop de  ; new process pointer
;
;	FIXME: do we actually *need* to restore the stack !
;
        push de ; restore stack
        push bc ; restore stack

	xor a
	out (21), a

	push de
        ld hl, #P_TAB__P_PAGE_OFFSET
	add hl, de	; process ptr
	pop de

        ld a, (hl)

	or a
	jr nz, not_swapped

	;
	;	We are still on the departing processes stack, which is
	;	fine for now.
	;
	ld sp, #_swapstack
	push hl
	push de
	call _swapper
	pop de
	pop hl
	ld a, (hl)

not_swapped:
	; Decide where the uarea stash lives right now and check the cache
	inc hl
	cp (hl)			; must cp first as (hl) vanishes on the out
	jr z, switchinlow
	ld hl, #_cached_page
	cp (hl)
	push af
	call z, update_cache
	pop af
	out (21), a
	exx			; thank goodness for exx 8)
	ld hl, #U_STASH_HIGH
	jr switchin_page
switchinlow:
	; Pages please !
	out (21), a
	exx			; thank goodness for exx 8)
	ld hl, #U_STASH_LOW
switchin_page:
        ; bear in mind that the stack will be switched now, so we can't use it
	; to carry values over this point
	ld de, #U_DATA
	ld bc, #U_DATA__TOTALSIZE
	ldir
	exx

	; Return to kernel mappings
	xor a
	out (21), a
        
switchlow2:

        ; check u_data->u_ptab matches what we wanted
        ld hl, (U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==IX
        jr nz, switchinfail

	; wants optimising up a bit
	ld ix, (U_DATA__U_PTAB)
        ; next_process->p_status = P_RUNNING
        ld P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING

	; Fix the moved page pointers
	; Just do two bytes as that is all we use on this platform
	ld l, P_TAB__P_PAGE_OFFSET(ix)
	ld h, P_TAB__P_PAGE_OFFSET+1(ix)
	ld (U_DATA__U_PAGE), hl
        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (U_DATA__U_SP)

        pop iy
        pop ix
        pop hl ; return code

        ; enable interrupts, if the ISR isn't already running
        ld a, (U_DATA__U_ININTERRUPT)
	ld (_int_disabled),a
        or a
        ret z ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
	call outhl
        ld hl, #badswitchmsg
        call outstring
	; something went wrong and we didn't switch in what we asked for
        jp _platform_monitor

; (hl) points to cached page ptr, a is desired page
update_cache:
	ld (hl), a
	ld hl, #0
	ld de, #0x8000
	ld bc, #0x7D00
	; map that page low (interrupts *must* be off)
	out (21), a
	ldir
	; put the kernel back
	xor a
	out (21), a
	ret

;
;	Invalidate a freed page - cache becomes void
;
_invalidate_cache:
	pop de
	pop hl
	push hl
	push de
	ld a, (_cached_page)
	cp l
	ret nz
	ld a, #0xff
	ld (_cached_page), a
	ret


_flush_cache:		; argument is the process it may apply to 
	pop de
	pop hl
	push hl
	push de
	ld a, (_int_disabled)
	push af
	ld de, #P_TAB__P_PAGE_OFFSET + 1
	add hl, de
	ld a, (_cached_page)
	cp (hl)
	jr nz, flush_none
	di
	call flush_cache_self
	pop af
	or a
	ret nz
	ei
	ret
flush_none:
	pop af
	ret

; interrupts must be disabled
flush_cache_self:
	push af
	ld a, (_cached_page)
	cp #0xff
	jr z, no_cache
	push bc
	push de
	push hl
	ld hl, #0x8000		; copy into the page
	ld de, #0
	ld bc, #0x7D00
	; map that page low (interrupts *must* be off)
	out (21), a
	ldir
	; put the kernel back
	xor a
	out (21), a
	pop hl
	pop de
	pop bc
no_cache:
	pop af
	ret

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        di ; should already be the case ... belt and braces.

        pop de  ; return address
        pop hl  ; new process p_tab*
        push hl
        push de

        ld (fork_proc_ptr), hl

        ; prepare return value in parent process -- HL = p->p_pid;
        ld de, #P_TAB__P_PID_OFFSET
        add hl, de
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        push hl ; HL still has p->p_pid from above, the return value in the parent
        push ix
        push iy

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
        ld (U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
	; process.
        ; --------- copy process ---------

        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        ; load p_page
        ld c, (hl)
	inc hl
	; load p_page + 1 (high page)
	ld b, (hl)
	push af
	ld a, c
	call outcharhex
	pop af
	; load existing page ptr
	ld a, (U_DATA__U_PAGE)

	call bankfork			;	do the bank to bank copy

	; Copy done

	ld a, (U_DATA__U_PAGE)	; parent memory
	out (21), a		; Switch context to parent

	; We are going to copy the uarea into the parents uarea stash
	; we must not touch the parent uarea after this point, any
	; changes only affect the child
	ld de, #U_STASH_LOW		; parent location
	ld hl, #U_DATA__U_PAGE
	ld a, (hl)
	inc hl
	cp (hl)
	jr z, stash_low
	ld de, #U_STASH_HIGH		; high stash
stash_low:
	ld hl, #U_DATA		; copy the udata from common
	ld bc, #U_DATA__TOTALSIZE
	ldir
	; Return to the kernel mapping
	xor a
	out (21), a
        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop bc
        pop bc
        pop bc

        ; Make a new process table entry, etc.
        ld  hl, (fork_proc_ptr)
        push hl
        call _newproc
        pop bc 

        ; runticks = 0;
        ld hl, #0
        ld (_runticks), hl
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
;	The parent must be running here, therefore the parent must be
;	mapped, therefore the cached page is loaded with the parent
;
bankfork:
	push af
	ld hl, #U_DATA__U_PAGE + 1
	cp (hl)			; Are we a two pager ?
	jr z, bankfork_low	; If not skip the high space
	call flush_cache_self	; Flush the parent cache
	ld a, b
	ld (_cached_page), a	; High cache is now child (which runs first)
	ld b, #0x80
	jr bankfork_go		; Now copy the low 32K
bankfork_low:
	ld b, #0x7D		; 32K minus UAREA stash
bankfork_go:
	pop af			; A is now the parent bank
	ld hl, #0		; base of memory to fork (vectors included)
bankfork_1:
	push bc			; Save our counter and also child offset
	push hl
	out (21), a		; switch to parent bank
	ld de, #bouncebuffer
	ld bc, #256
	ldir			; copy into the bounce buffer
	pop de			; recover source of copy to bounce
				; as destination in new bank
	pop bc			; recover child page number
	push bc
	ld b, a			; save the parent bank id
	ld a, c			; switch to the child
	out (21), a
	push bc			; save the bank pointers
	ld hl, #bouncebuffer
	ld bc, #256
	ldir			; copy into the child
	pop bc			; recover the bank pointers
	ex de, hl		; destination is now source for next bank
	ld a, b			; parent bank is wanted in a
	pop bc
	djnz bankfork_1		; rinse, repeat
	ret

;
;	For the moment
;
bouncebuffer:
	.ds 256
;
;	We can keep a stack in common because we will complete our
;	use of it before we switch common block. In this case we have
;	a true common so it's even easier. This can share with the bounce
;	buffer used by bankfork as we won't switchin mid way through the
;	banked fork() call.
;
_swapstack:
_cached_page:
	.db 0xff
