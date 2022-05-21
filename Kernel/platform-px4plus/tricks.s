; 2013-12-21 William R Sowerbutts

        .module tricks

        .globl _ptab_alloc
        .globl _newproc
        .globl _chksigs
        .globl _getproc
        .globl _plt_monitor
        .globl _inint
        .globl _plt_switchout
        .globl _switchin
        .globl _doexec
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
	.globl _swapper
	.globl _swapout

	.globl _map_translate
	.globl _map_live
	.globl _sidecar_copy
	.globl _cartridge_copy
	.globl _make_mapped

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

        ld hl, #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
        push ix
        push iy
        ld (_udata + U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

        ; find another process to run (may select this one again)
	push af
        call _getproc
	pop af

        push hl
	push af
        call _switchin
	pop af

        ; we should never get here
        call _plt_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
            .db 13, 10, 0

_switchin:
        di
	pop hl
        pop bc  ; return address
        pop de  ; new process pointer
;
;	FIXME: do we actually *need* to restore the stack !
;
        push de ; restore stack
        push bc ; restore stack
	push hl

	push de
        ld hl, #P_TAB__P_PAGE_OFFSET
	add hl, de	; process ptr
	pop de

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
	;	Let the swapper run
	;
	push hl
	push de
	push af
	call _swapper
	pop af
	pop de
	pop hl
	ld a, (hl)

not_swapped:
	;
	; 	On the PX4 this is only half the game. The process we want
	;	is probably stuffed into the sidecar or cartridge
	;
	;	Make bank A mapped bank. Preserves DE, trashes AF,BC,HL
	;
	call _make_mapped
	;
        ; check u_data->u_ptab matches what we wanted
	;
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==IX
        jr nz, switchinfail

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

        ; enable interrupts, if the ISR isn't already running
        ld a, (_udata + U_DATA__U_ININTERRUPT)
        or a
        ret nz ; in ISR, leave interrupts off
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

	pop bc
        pop de  ; return address
        pop hl  ; new process p_tab*
        push hl
        push de
	push bc

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

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        ; --------- copy process ---------

        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        ; load p_page
        ld c, (hl)
	; load existing page ptr
	push af
	ld a, c
	call outcharhex
	pop af
	ld a, (_udata + U_DATA__U_PAGE)

	call bankfork			;	do the bank to bank copy

	; Fix up stack

        pop bc
        pop bc
        pop bc

        ; Make a new process table entry, etc.
        ld  hl, (fork_proc_ptr)
        push hl
	push af
        call _newproc
	pop af
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
;	A is the parent C is the child bank. Both of these are of course
;	logical banks. The logical bank of A right now is 1 (in memory),
;	but this will all change
;
;	We do a copy to the I/O mapped sidecar or cartridge rather than the
;	usual LDIR. We also update our logical mapping table as we
;	effectively swapped ram between parent and child.
;
bankfork:
	push af
	ld b, #0
	ld hl, #_map_translate
	add hl, bc
	push hl
	ld a, (hl)
	bit 7, a
	jr nz, copy_to_sidecar
	;
	; Forking to cartridge
	;
	call _cartridge_copy
	jr bankcopy_done
copy_to_sidecar:
	call _sidecar_copy
bankcopy_done:
	pop hl		; child translation entry
	ld a, (hl)	; entry that was allocated
	ld (hl), #1	; child is now main memory
	ld de, (_map_live)	; parent entry
	ld (de), a		; parent now gets "new" entry as child has
				; the RAM
	ld (_map_live), hl	; live task map
	ld b, #0
	pop af
	ret
;
;	We can keep a stack in common because we will complete our
;	use of it before we switch common block. In this case we have
;	a true common so it's even easier.
;
	.ds 128
_swapstack:
