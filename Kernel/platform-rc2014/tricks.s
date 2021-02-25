        .module tricks

        .include "kernel.def"
        .include "../kernel-z80.def"

TOP_PORT	.equ	MPGSEL_3

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
	.globl _ramtop
	.globl _need_resched
	.globl top_bank
	.globl _int_disabled
	.globl _udata
	.globl _kernel_pages
	.globl map_kernel_restore
	.globl _get_common
	.globl _swap_finish

	.globl ldir_or_dma

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .area _COMMONMEM

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.dw 0
_need_resched:
	.db 0

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_platform_switchout:
        ; save machine state

        ld hl, #0 ; return code set here is ignored, but _switchin can
        ; return from either _switchout OR _dofork, so they must both write
        ; _udata + U_DATA__U_SP with the following on the stack:
        push hl ; return code
	ld hl,(_kernel_pages + 1)	; Save the kernel banks
	push hl
        push ix
        push iy
        ld (_udata + U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

        ; find another process to run (may select this one again)
	push af
        call _getproc
	pop af

        push hl
	push hl
        call _switchin

        ; we should never get here
        call _platform_monitor

badswitchmsg: .ascii "_switchin: FAIL"
        .db 13, 10, 0

_switchin:
        di
	pop hl	; bank
        pop bc  ; return address
        pop de  ; new process pointer
;
;	FIXME: do we actually *need* to restore the stack !
;
        push de ; restore stack
        push bc ; restore stack
	push hl ; bank

        ld hl, #P_TAB__P_PAGE_OFFSET	; Base page number or swap info
	add hl, de		; process ptr
	ld a, (hl)
	or a
	jr nz, notswapped

	;
	;	Swap us in (may swap someone else out)
	;	Hands us a new common in L
	;
	;	FIXME: look at interrupt disable/enable logic here
	;
	push de
	push hl			; We need these in a bit

	push af
	call _get_common
	pop af

	ld a,l
	;
	;	Recover saved registers before we stack switch
	;
	pop hl
	pop de
	;
	;	Switch to our new common
	;
	out (TOP_PORT),a	; Map in our common and udata
	ld (top_bank),a
	;
	;	Use a temporary stack
	;
	ld sp, #swapstack
	;
	;	Save back onto that instead
	;
	push hl
	push de

	;
	;	swap_finish(page, process)
	;
	push de			; process
	push af			; page for common
	inc sp
	push af
	call _swap_finish
	pop af
	pop af
	inc sp

	;	Recover registers we need
	pop de
	pop hl
notswapped:
	inc hl
	inc hl
	inc hl			; common page
	ld a,(hl)
	out (TOP_PORT), a	; *CAUTION* our stack just left the building
	ld (top_bank),a

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
	pop hl
	ld (_kernel_pages + 1),hl
	call map_kernel_restore
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
	ld hl,(_kernel_pages + 1)
	push hl
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
        pop bc

        ; The child makes its own new process table entry, etc.
	ld hl, #_udata
	push hl
        ld hl, (fork_proc_ptr)
        push hl
	push af
        call _makeproc
	pop af
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


        .area _COMMONMEM

	.globl mpgsel_cache

fork_copy:
	ld hl, (_udata + U_DATA__U_TOP)
	ld de, #0x0fff
	add hl, de		; + 0x1000 (-1 for the rounding to follow)
	ld a, h
	rlca
	rlca			; get just the number of banks in the bottom
				; bits
	and #3
	inc a			; and round up to the next bank
	ld b, a
	; we need to copy the relevant chunks
	ld hl, (fork_proc_ptr)
	ld de, #P_TAB__P_PAGE_OFFSET
	add hl, de
	; hl now points into the child pages
	ld de, #_udata + U_DATA__U_PAGE
	; and de is the parent
fork_next:
	ld a,(hl)
	out (MPGSEL_1), a	; 0x4000 map the child
	ld c, a
	inc hl
	ld a, (de)
	out (MPGSEL_2), a	; 0x8000 maps the parent
	inc de
	exx
	ld hl, #0x8000		; copy the bank
	ld de, #0x4000
	ld bc, #0x4000		; we copy the whole bank, we could optimise
				; further
	call ldir_or_dma	; use Z80DMA if we can
	exx
	call map_kernel		; put the maps back so we can look in p_tab
	djnz fork_next
	ld a, c
	ld (mpgsel_cache+3),a	; cache the page number
	out (MPGSEL_3), a	; our last bank repeats up to common
	; --- we are now on the stack copy, parent stack is locked away ---
	ret			; this stack is copied so safe to return on

	
	.ds 256
swapstack:
