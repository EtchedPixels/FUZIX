
	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

	;
	; We can't quite use the standard helpers as we have rather weird
	; memory mapping logic
	;
        .module cpcsmetricks

        .globl _ptab_alloc
        .globl _makeproc
        .globl _chksigs
        .globl _getproc
        .globl _plt_monitor
        .globl _plt_switchout
        .globl _switchin
        .globl _doexec
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
	.globl _need_resched
	.globl _nready
	.globl _plt_idle
	.globl _int_disabled
	.globl _udata

	.globl ldir_to_user
	.globl ldir_far
	.globl a_map_to_bc

	.globl _vtborder

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .area _COMMONMEM

; __switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
; 
; This function can have no arguments or auto variables.
;
_plt_switchout:
        ld hl, #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
		push ix
        push iy
        ld (_udata + U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

	; Stash the uarea back into process memory
	ld hl, #_udata
	ld ix, #U_DATA_STASH
	ld bc, #U_DATA__TOTALSIZE
	; Slow - must be a more elegant way to tackle this ! FIXME
	call ldir_to_user

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _plt_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
            .db 13, 10, 0

_switchin:
        di
        pop bc  ; return address
        pop de  ; new process pointer

        ld hl, #P_TAB__P_PAGE_OFFSET
	add hl, de	; process ptr

.ifne CONFIG_SWAP
	.globl _swapper
	;
	;	Always use the swapstack, otherwise when we call map_kernel
	;	having copied the udata stash back to udata we will crap
	;	somewhere up the stackframe and it's then down to luck
	;	if those bytes are discarded or not.
	;
	;	Yes - this was a bitch to debug, please don't break it !
	;
	ld sp, #_swapstack

        ld a, (hl)

	or a
	jr nz, not_swapped

	;
	;	Re-enable interrupts while we swap. This is ok because
	;	we are not on the IRQ stack when switchin is invoked.
	;
	;	There are two basic cases
	;	#1: pre-emption. Not in a system call, must avoid
	;	re-entering pre-emption logic, Z80 lowlevel code sets U_INSYS
	;	#2: kernel syscall. Also protected by U_DATA__U_INSYS
	;
	ei
	xor a
	ld (_int_disabled),a
	push hl
	push de
	call _swapper
	pop de
	pop hl
	ld a,#1
	ld (_int_disabled),a
	di
.endif
	ld a, (hl)
not_swapped:
	; We need the swap stack anyway or we run out of registers
	ld sp, #_swapstack

	ld hl, (_udata + U_DATA__U_PTAB)
	or a
	sbc hl, de
	jr z, skip_copyback	; Tormod's optimisation: don't copy the
				; the stash back if we are the task who
				; last owned the real udata

        ; bear in mind that the stack will be switched now, so we can't use it
	; to carry values over this point

	push de

	; Trick - we don't overlay the udata proper with anything so we can
	; map common as write, process as read and ldir
	; Or we could expect it blows up the emulator so do it the slow way
	; for now
;	or #0x80
;	out (0x40),a
;	ld hl, #U_DATA_STASH
;	ld de, #_udata
;	ld bc, #U_DATA__TOTALSIZE
;	ldir
;	ld a,#1		; back to kernel
;	out (0x40),a

	ld hl, # U_DATA_STASH
	ld ix, #_udata
	ld bc, #U_DATA__TOTALSIZE
	ld d,a
	ld e,#0xc2
	call ldir_far

	pop de

	; In the non swap case we must set sp before we use the stack
	; otherwise we risk corrupting the restored stack frame
        ld sp, (_udata + U_DATA__U_SP)

        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==HL
        jr nz, switchinfail

skip_copyback:
	; wants optimising up a bit
	ld ix, (_udata + U_DATA__U_PTAB)
        ; next_process->p_status = P_RUNNING
        ld P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING

	; Fix the moved page pointers
	; Just do one byte as that is all we use on this platform
	ld a, P_TAB__P_PAGE_OFFSET(ix)
	ld (_udata + U_DATA__U_PAGE), a
        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (_udata + U_DATA__U_SP)

        pop iy
        pop ix
        pop hl ; return code

	; enable interrupts, if we didn't pre-empt in an ISR
        ld a, (_udata + U_DATA__U_ININTERRUPT)
	ld (_int_disabled),a
        or a
        ret nz ; Not an ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
	call outhl
        ld hl, #badswitchmsg
        call outstring
	; something went wrong and we didn't switch in what we asked for
        jp _plt_monitor

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        di ; should already be the case ... belt and braces.

	;
	; FIXME: we should no longer need interrupts off for most of a
	; fork() call.
	;
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

        ld (_udata + U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
	; process.

        ; --------- copy process ---------

        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        ; load p_page
        ld a, (hl)
		call a_map_to_bc	;BC->child bank for bankfork
	; load existing page ptr
	ld a, (_udata + U_DATA__U_PAGE)
	push bc
	exx
	push bc			;store BC'
	call a_map_to_bc ;BC'->parent bank for bankfork
	exx
	call bankfork			;	do the bank to bank copy
	exx
	pop bc			;restore BC'
	exx
	pop bc
	; Copy done

	; We are going to copy the uarea into the parents uarea stash
	; we must not touch the parent uarea after this point, any
	; changes only affect the child
	ld hl, #_udata		; copy the udata from common into the
	ld ix, #U_DATA_STASH	; target process
	ld bc, #U_DATA__TOTALSIZE

	call ldir_to_user

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop iy
        pop ix
        pop hl

        ; Make a new process table entry, etc.
	ld hl,#_udata
	push hl
        ld  hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop bc 
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

	.area _COMMONMEM
;
;	We can keep a stack because we don't have enough registers
;	to do all the bank trickery we need
;
	.ds 256
_swapstack:
_need_resched:	.db 0

;
;	Fork has a special case fast copier. We need to optimize ldir_far
;	as well but fork has the special property that source == dest in
;	differing banks and that makes a huge speed difference
;
;	We are swapping the full address space so we must be really careful
;	that we save and restore globals in the same bank!
;
bankfork:	;BC parent, BC' child

	push bc
	ld bc,#0x7f10
	out (c),c
	ld c,#0x4e  ;orange
	out (c),c
	pop bc
	out (c),c		;->switch to child bank
	exx
	ld (cpatch2 + 1),bc	; patch parent into loops ->child bank
	ld (spcache),sp		;->child bank
		; 15 outer loops
	ld a,#15
	ld (copyct),a	;->child bank
	;
	;	Set up ready for the copy
	;
	; F000 bytes to copy.
	; Stack pointer at the target buffer
	out(c),c	;->switch to parent bank
	exx
	ld (cpatch1 + 1),bc	; patch child into loop ->parent bank
	ld sp,#PROGBASE	; Base of memory to fork
	xor a
	ex af,af'	; Save A as we need A for data transfer
	
	; Each loop takes 112 cycles to read 16 bytes and save sp
	; 117 cycles to switch bank and write them
	; 56 cycles do switch bank back and do housekeeping
	; and a few more per 4K block
	;
	; That comes in at 17 cycles/byte which is only one clock/byte
	; slower than a non banking LDIR
copyloop:
	pop bc		; copy 16 bytes out of parent
	pop de
	pop hl
	exx
	pop de
	pop hl
	pop ix
	pop iy
	pop af
	; We patch in parent bank we must therefore read in parent bank
	ld (sp_patch+1),sp 
cpatch1:		;this is executed from parent bank
	ld bc,#0		; child bank (also patched in for speed)
	out (c),c
	push af		; and put them back into the child
	push iy	
	push ix
	push hl
	push de
	exx
	push hl
	push de
	push bc
	ex af,af'	; Get counter back
	dec a
	jr z, setdone	; 256 loops ?
copy_cont:
	; Switch back to parent bank so that we get the right sp_patch
	ex af,af'
cpatch2:	;this is executed from child bank
	ld bc,#0
	out (c),c
sp_patch:	;this is executed from parent bank
	ld sp,#0
	jp copyloop
;
;	This outer loop only runs 8 times so isn't quite so performance
;	critical
;
setdone:
	; We count down in the child bank context
	ld hl,#copyct
	dec (hl)	
	jr z, copy_over
	ld a,#0
	jr copy_cont
copy_over:
	;
	;	Get the stack back
	;
	ld sp,(spcache)
	;
	;	And the correct kernel bank.
	;
	ld bc,#0x7fc2
	out (c),c
	ld bc,#0x7f10
	out (c),c                                    
	ld a,(_vtborder)
	out (c),a
	ret

spcache:	;this is read from child bank
	.word 0
copyct:		;this is read from child bank
	.byte 0
