; 2013-12-21 William R Sowerbutts

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
	.globl map_kernel
	.globl _need_resched
	.globl _int_disabled
	.globl _udata

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .include "kernel.def"
        .include "../kernel-z80.def"

        .area _COMMONMEM

_need_resched:
	.db 0

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; This function can have no arguments or auto variables.
_platform_switchout:
        ; save machine state

        ld hl, #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
        push ix
        push iy
        ld (_udata + U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _platform_monitor

badswitchmsg: .ascii "_switchin: FAIL"
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

        ld hl, #P_TAB__P_PAGE_OFFSET
	add hl, de		; process ptr
	ld a,(hl)
	inc hl
	ld h,(hl)		; page pointer
	ld l,a

	ld bc,#7
	add hl,bc		; common

	ld bc,#0x243B
	ld a,#0x57		; common bank MMU pointer
	out (c),a

	ld a, (hl)
	inc b
	out (c), a	; *CAUTION* our stack just left the building

	; ------- No stack -------
        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==IX
        jr nz, switchinfail

	; wants optimising up a bit
	ld hl, #P_TAB__P_STATUS_OFFSET
	add hl, de
	ld (hl), #P_RUNNING

        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (_udata + U_DATA__U_SP)

	; ---- New task stack ----

        pop iy
        pop ix
        pop hl ; return code

        ; enable interrupts, if the ISR isn't already running
        ld a, (_udata + U_DATA__U_ININTERRUPT)
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
        ld (_udata + U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	; --------- we switch stack copies in this call -----------
	call fork_copy			; copy 0x000 to ptab.p_top and the
					; uarea and return on the childs
					; common
	; We are now in the kernel child context

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop bc
        pop bc
        pop bc

        ; The child makes its own new process table entry, etc.
	ld hl, #_udata
	push hl
        ld hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop bc 
	pop bc

	; any calls to map process will now map the childs memory

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
;	FIXME: DMA for this ?
;
fork_copy:
	ld hl, (fork_proc_ptr)
	ld de, #P_TAB__P_PAGE_OFFSET
	add hl,de
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	ex de,hl

	; DE is now the 8 page pointers for the child

	ld hl, (_udata + U_DATA__U_PAGE)	; pointer to banks of parent

	; HL for the parent

	ld b,#7			; 7 full banks to copy

	ld a,#0xff		; never used page value
fork_copy_loop:
	cp (hl)			; same as previous - don't copy
	jr z, nocopy
	ld a,(hl)
	push bc
	call do_copy
	pop bc
nocopy:
	inc hl
	inc de
	djnz fork_copy_loop

	exx
	ld bc,#0x0200		; 512 bytes at 0xF000
				; will need changes if we move it!
	ld hl,#0x3000
	ld de,#0x5000
	call do_partial_copy
	;
	; Put the MMU mappings back to sanity
	;
	jp map_kernel

do_copy:
	; On entry (HL) is the source bank (DE) is the target bank
	; BC is the size
	;
	; We preserve AF, DE, HL
	exx
	ld hl,#0x2000
	ld de,#0x4000
	ld bc,#0x2000
do_partial_copy:
	exx
	push af
	ld bc,#0x243B
	ld a,#0x51
	out (c),a		; 2000-4000
	inc b
	ld a,(hl)
	out (c),a
	dec b
	ld a,#0x52		; 4000-6000
	out (c),a
	inc b
	ld a,(de)
	out (c),a
	pop af
	exx
	ldir
	exx
	ret
