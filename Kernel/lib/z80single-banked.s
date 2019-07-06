;
;	Generic handling for single process in memory Z80 systems.
;
;	This code could really do with some optimizing.
;
;	FIXME: IRQ enable logic during swap
;
        .module tricks

        .globl _ptab_alloc
        .globl _makeproc
        .globl _chksigs
        .globl _getproc
        .globl _platform_monitor
        .globl _platform_switchout
        .globl _switchin
        .globl _doexec
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
	.globl _swapper
	.globl _swapout
	.globl _swapout_new
	.globl _int_disabled
	.globl _udata
	.globl _tmpbuf
	.globl _tmpfree

	.globl map_save_kmap
	.globl map_restore_kmap

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .area _COMMONMEM

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; This function can have no arguments or auto variables.
_platform_switchout:
        di
        ; save machine state

        ld hl, #0 ; return code set here is ignored, but _switchin can 

        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
        push ix
        push iy
	call map_save_kmap
	push af
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
	push af
        call _platform_monitor
	pop af

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
            .db 13, 10, 0

_switchin:
        di
	ld a,#1
	ld (_int_disabled),a
	pop hl	; bank
        pop bc  ; return address
        pop de  ; new process pointer

        push de ; restore stack
        push bc ; restore stack
	push hl

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
	;
	;	The process we are switching from is not resident. In
	;	practice that means it exited. We don't need to write
	;	it back out as it's dead.
	;
	ld ix,(_udata + U_DATA__U_PTAB)
	ld a,P_TAB__P_PAGE_OFFSET(ix)

	or a
	jr z, not_resident
	;
	;	DE = process we are switching to
	;
	push de	; save process
	; We will always swap out the current process
	ld hl, (_udata + U_DATA__U_PTAB)
	push hl
	push af
	call _swapout
	pop af
	pop hl
	pop de	; process to swap to
not_resident:
	push de	; called function may modify on stack arg so make
	push de	; two copies
	push af
	call _swapper
	pop af
	pop de
	pop de

not_swapped:        
        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==HL
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

	pop af
        pop iy
        pop ix
        pop hl ; return code

	call map_restore_kmap

        ; enable interrupts, if the ISR isn't already running
        ld a, (_udata + U_DATA__U_ININTERRUPT)
	; put th einterrupt status flag back correctly
	ld (_int_disabled),a
        or a
        ret nz ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
	call outhl
        ld hl, #badswitchmsg
        call outstring
	; something went wrong and we didn't switch in what we asked for
        jp _platform_monitor


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

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
	ld hl,#0
        push hl		;	#0 child
        push ix
        push iy
	call map_save_kmap
	push af

        ; save kernel stack pointer -- when it comes back in the child we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
	; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
        ld (_udata + U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
	; process.


        ; Make a new process table entry, etc.

	; Copy the parent properties into the temporary udata copy
	push af
	call _tmpbuf
	pop af
	ld a,h
	or l
	jp z,ohpoo
	push hl
	ex de,hl
	ld hl, #_udata
	ld bc, #U_DATA__TOTALSIZE
	ldir

	; Recover the buffer pointer
	pop hl
	push hl

	; Make the child udata out of the temporary buffer
	push hl
        ld hl, (fork_proc_ptr)
        push hl
	push af
        call _makeproc
	pop af
        pop bc
	pop bc

        ; in the child process, fork() returns zero.
	;
	; And we exit, with the kernel mapped, the child assembled in the
	; copy area as if it had done a switchout, the parent meanwhile
	; continues happily on

        ; now we're in a safe state for _switchin to return in the child
	; process swap out the image and the new udata

	; Stack the buffer as a second argument
	pop hl
	push hl
	push hl

	ld hl, (fork_proc_ptr)
	push hl
	push af
	call _swapout_new
	pop af
	pop hl
	pop hl

	; Buffer pointer is sitting top of stack still
	; so use it as an argument to tmpfree
	push af
	call _tmpfree
	pop af

	pop hl

	ld hl, (fork_proc_ptr)
        ; prepare return value in parent process -- HL = p->p_pid;
        ld de, #P_TAB__P_PID_OFFSET
        add hl, de
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a
        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop bc
        pop bc
        pop bc
	pop bc
	; Return pid of child we forked into swap
        ret

ohpoo:
	ld hl,#nobufs
	call outstring
	jp _platform_monitor

nobufs:
	.asciz 'nobufs'

	.area _COMMONDATA
;
;	We can keep a stack in common because we will complete our
;	use of it before we switch common block. In this case we have
;	a true common so it's even easier.
;
	.ds 128
_swapstack:

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
