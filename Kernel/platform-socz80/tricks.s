; 2013-12-21 William R Sowerbutts
; 2015-01-30 Alan Cox

        .module tricks

        .globl _ptab_alloc
        .globl _makeproc
	.globl _udata
        .globl _chksigs
        .globl _getproc
        .globl _plt_monitor
        .globl _plt_switchout
        .globl _switchin
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
	.globl _int_disabled

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .include "socz80.def"
        .include "kernel.def"
        .include "../kernel-z80.def"
;
;	These do not need to be in common memory as we don't have to
;	pull udata remapping tricks on a 16K banked system. In the case of
;	SocZ80 they *MUST NOT* be in common as they reload the MMU page for
;	0xF000. That takes multiple instructions and the code will vanish
;	midstream if it is in common space!
;
	.area _CODE2

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
; 
; This function can have no arguments or auto variables.
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
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _plt_monitor

_switchin:
        di
        pop bc  ; return address
        pop de  ; new process pointer
        push de ; restore stack
        push bc ; restore stack

        ld iy, #0
        add iy, de

	; keep new process pointer in DE so we can compare with
	; it after the switch

	; Get our page pointer for the top bank
        ld l, P_TAB__P_PAGE_OFFSET+3(iy)
	; Convert it into an MMU frame
	sla l
	rla
	sla l
	rla
	and #3
	ld h, a
	; We want the very top frame
	inc l
	inc l
	inc l

        ; bear in mind that the stack will be switched now, so we can't use it
	; to carry values over this point
        
        ; get next_process->p_page value and load it into the MMU for the
	; top page
        ld a, #0x0f
        out (MMU_SELECT), a
        ld a, h
        out (MMU_FRAMEHI), a
        ld a, l
        out (MMU_FRAMELO), a

	; ------------- Our stack just left the building ---------------

        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==HL
        jr nz, switchinfail

        ; next_process->p_status = P_RUNNING
        ld P_TAB__P_STATUS_OFFSET(iy), #P_RUNNING

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
	ld (_int_disabled),a
        or a
        ret nz ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
        ld hl, #badswitchmsg
        call outstring
	; something went wrong and we didn't switch in what we asked for
        jp _plt_monitor


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

	;
	; We will return on the parent stack with everything copied. We can
	; thus return safely down the stack as we switch stacks below
	;

	call fork_copy

	;
        ; switch into the child process context
        ; get address of top page
	;
        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET + 3
        add hl, de
        ; load p_page[3]
        ld l, (hl)
	ld h, #0
	add hl, hl
	add hl, hl
	inc hl		; last one of the group
	inc hl
	inc hl
	push hl
	call outhl
	pop hl
        ; load into MMU
        ld a, #0x0F
        out (MMU_SELECT), a
        ld a, h
        out (MMU_FRAMEHI), a
        ld a, l
        out (MMU_FRAMELO), a

	ld a,#'!'
	call outchar

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop bc
        pop bc
        pop bc

	ld hl,#_udata
	push hl
        ; Make a new process table entry, etc.
        ld  hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop bc 
	pop bc

        ; runticks = 0;
        ld hl, #0
        ld (_runticks), hl

        ; in the child process, fork() returns zero.
        ret


fork_copy:
	; child mapping
	ld hl, (fork_proc_ptr)
	ld de, #P_TAB__P_PAGE_OFFSET
	add hl, de
	; parent mapping to be copied
	ld de, #_udata + U_DATA__U_PAGE

;
;	Walk the 16K banks using a generic code pattern
;	bank_copy is invoked with registers saved and the
;	parent and child in a nd c
;

	ld a, #4
	ld b, #0		; not a valid page
nextbank:
	push af
	ld c, (hl)
	inc hl
	ld a, (de)
	inc de
	cp b
	jr z, done		; repeated map, means all we need is copied
	ld b, a
	push bc
	push de
	push hl
	call bank_copy		; copies the 16K
	pop hl
	pop de
	pop bc
	pop af
	dec a
	jr nz, nextbank	
	ret
done:
	pop af
	ret


;
;	Copy 16K from bank b to bank a (as seen by the software 16K paging).
;	In fact we need to move blocks of four pages.
;
bank_copy:

	; save the parent mapping
	ld b, a
        ; store the old MMU mapping
        ld a, #0x0E
        out (MMU_SELECT), a
        in a, (MMU_FRAMEHI)
        ld d, a
        in a, (MMU_FRAMELO)
        ld e, a
        push de
        ld a, #0x0D
        out (MMU_SELECT), a
        in a, (MMU_FRAMEHI)
        ld d, a
        in a, (MMU_FRAMELO)
        ld e, a
        push de

        ; Copy a 16K bank

        ; child base page is in c parent in b
	ld d,#0
	ld l, d
	ld a, c
	sla a
	rl d
	sla a
	rl d
	ld e, a
	ld h,#0x10     ; +16MB, this puts the destination pointer into uncached DRAM.
        add hl, de     ; this is so we don't obliterate the read cache (the cache is
                       ; direct mapped and so the parent and child processes are cache
                       ; aliases of each other)

	ld a, b	       ; parent
	ld d, #0
	sla a
	rl d
	sla a
	rl d           ; de now holds the src pointer
	ld e, a

        ld b, #4 ; we're going to copy 16K
copynextpage:
        ; map source page at E000
        LD a, #0x0E
        out (MMU_SELECT), a
        ld a, d
        out (MMU_FRAMEHI), a
        ld a, e
        out (MMU_FRAMELO), a
        ; map destination page at D000
        ld a, #0x0D
        out (MMU_SELECT), a
        ld a, h
        out (MMU_FRAMEHI), a
        ld a, l
        out (MMU_FRAMELO), a

        push hl
        push de
        push bc
	call outde
	call outhl

        ld hl, #0xe000
        ld de, #0xd000
        ld bc, #0x1000
        ldir

        pop bc
        pop de
        pop hl
        
        ; next loop, do the next 4k
        inc hl
        inc de

        ; loop around
        djnz copynextpage

        ; done copying, restore MMU
        ld a, #0x0D
        out (MMU_SELECT), a
        pop bc
        ld a, b
        out (MMU_FRAMEHI), a
        ld a, c
        out (MMU_FRAMELO), a
        ld a, #0x0E
        out (MMU_SELECT), a
        pop bc
        ld a, b
        out (MMU_FRAMEHI), a
        ld a, c
        out (MMU_FRAMELO), a

	ret

	.area _DATA
fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
	.area _CONST
badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
