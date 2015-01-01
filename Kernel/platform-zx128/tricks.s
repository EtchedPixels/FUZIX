; 2013-12-21 William R Sowerbutts
; TODO: this code is copied from z80pack. Need to rewrite for zx128.

        .module tricks

        .globl _ptab_alloc
        .globl _newproc
        .globl _chksigs
        .globl _getproc
        .globl _trap_monitor
        .globl trap_illegal
        .globl _inint
        .globl _switchout
        .globl _switchin
        .globl _doexec
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
        .globl dispatch_process_signal

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .include "kernel.def"
        .include "../kernel.def"

        .area _COMMONMEM

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; FIXME: make sure we optimise the switch to self case higher up the stack!
; 
; This function can have no arguments or auto variables.
_switchout:
        di
        call _chksigs
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
	ld bc, #0x7ffd
	out (c), a
	ld hl, #U_DATA
	ld de, #U_DATA_STASH
	ld bc, #U_DATA__TOTALSIZE
	ldir
	xor a
	ld bc, #0x7ffd
	out (c), a

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _trap_monitor

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
	ld bc, #0x7ffd
	out (c), a

	push de
        ld hl, #P_TAB__P_PAGE_OFFSET
	add hl, de	; process ptr
	pop de

        ld a, (hl)
	; Pages please !
	out (c), a      ; BC still contains 0x7ffd

        ; bear in mind that the stack will be switched now, so we can't use it
	; to carry values over this point

	exx			; thank goodness for exx 8)
	ld hl, #U_DATA_STASH
	ld de, #U_DATA
	ld bc, #U_DATA__TOTALSIZE
	ldir
	exx

	xor a
	out (c), a      ; and again 0x7ffd in BC
        
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
	; Just do one byte as that is all we use on this platform
	ld a, P_TAB__P_PAGE_OFFSET(ix)
	ld (U_DATA__U_PAGE), a
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
        ld a, (_inint)
        or a
        ret z ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
	call outhl
        ld hl, #badswitchmsg
        call outstring
	; something went wrong and we didn't switch in what we asked for
        jp _trap_monitor

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

	; Need to write a new 16K bank copy here, then copy the live uarea
	; into the stash of the new process

        ; --------- copy process ---------

        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        ; load p_page
        ld c, (hl)
	ld hl, (U_DATA__U_PAGE)
	ld a, l

	call bankfork			;	do the bank to bank copy

	; Copy done

	ld hl, (U_DATA__U_PAGE)	; parent memory
        ld a, l
	ld bc, #0x7ffd
	out (c), a		; Switch context to parent

	; We are going to copy the uarea into the parents uarea stash
	; we must not touch the parent uarea after this point, any
	; changes only affect the child
	ld hl, #U_DATA		; copy the udata from common into the
	ld de, #U_DATA_STASH	; target process
	ld bc, #U_DATA__TOTALSIZE
	ldir

	ld bc, #0x7ffd
	xor a
	out (c), a
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
;
;	Note: this needs reviewing. We now have a lot more program memory
;	we can use with a lazy copying model
;
bankfork:
;	ld bc, #(0x4000 - 768)		;	16K minus the uarea stash

	ld b, #0x3D		; 40 x 256 minus 3 sets for the uarea stash
bankfork_1:
	ld hl, #0xC000		; base of memory to fork (vectors included)
	push bc			; Save our counter and also child offset
	push hl
	ld bc, #0x7ffd
	out (c), a		; switch to parent bank
	ld de, #bouncebuffer
	ld bc, #256
	ldir			; copy into the bounce buffer
	pop de			; recover source of copy to bounce
				; as destination in new bank
	pop bc			; recover child port number
	push bc
	ld b, a			; save the parent bank id
	ld a, c			; switch to the child
	push bc			; save the bank pointers
	ld bc, #0x7ffd
	out (c), a
	ld hl, #bouncebuffer
	ld bc, #256
	ldir			; copy into the child
	pop bc			; recover the bank pointers
	ex de, hl		; destination is now source for next bank
	ld a, b			; parent back is wanted in a
	pop bc
	djnz bankfork_1		; rinse, repeat
	ret

;
;	For the moment
;
	.area _COMMONDATA
bouncebuffer:
	.ds 256
;	We can keep a stack in common because we will complete our
;	use of it before we switch common block. In this case we have
;	a true common so it's even easier. We never use both at once
;	so share with bouncebuffer
_swapstack:
