; 2013-12-21 William R Sowerbutts

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
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl mmu_map_page
        .globl map_process
        .globl mmu_state_dump
        .globl mmu_map_page_fast
        .globl interrupt_handler

        ; imported debug symbols
        .globl outstring, outde, outhl, outbc, outnewline, outchar, outcharhex

        .include "socz80.def"
        .include "../kernel.def"
        .include "kernel.def"

        .area _COMMONMEM

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
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

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _trap_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
_switchin:
        di
        pop bc  ; return address
        pop de  ; new process pointer
        push de ; restore stack
        push bc ; restore stack

        ld iy, #0
        add iy, de

        ld h, P_TAB__P_PAGE_OFFSET+1(iy)
        ld l, P_TAB__P_PAGE_OFFSET(iy)
        push de
        ld de, #0x0f
        add hl, de
        pop de ; keep new process pointer in DE so we can compare with
	       ; it after the switch

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

        ; check u_data->u_ptab matches what we wanted
        ld hl, (U_DATA__U_PTAB) ; u_data->u_ptab
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
        ld hl, #badswitchmsg
        call outstring
	; something went wrong and we didn't switch in what we asked for
        call nz, _trap_monitor

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

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
        ; store the old MMU mapping
        ld a, #0x0E
        out (MMU_SELECT), a
        in a, (MMU_FRAMEHI)
        ld b, a
        in a, (MMU_FRAMELO)
        ld c, a
        push bc
        ld a, #0x0D
        out (MMU_SELECT), a
        in a, (MMU_FRAMEHI)
        ld b, a
        in a, (MMU_FRAMELO)
        ld c, a
        push bc

        ; Copy entire 64k process into Child's address space
        ; note source base page is in DE
        ; dest base page is in (_ub+OPAGE) which is in 0xF000, which will not
	; be available after we start remapping
        ; we can do the copy in frame 0xE000 and 0xF000

        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        ; load child base page
        ld e, (hl)
        inc hl
        ld d, (hl)
        ld hl, #0x1000 ; +16MB, this puts the destination pointer into uncached DRAM.
        add hl, de     ; this is so we don't obliterate the read cache (the cache is
                       ; direct mapped and so the parent and child processes are cache
                       ; aliases of each other)

        ld de, (U_DATA__U_PAGE) ; load parent page base page

        ld b, #16 ; we're going to copy 0x0000 ... 0xFFFF
copynextpage:
        ; map source page at E000
        LD A, #0x0E
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
        ; --------- copy completed ---------

        ; switch into the child process context
        ; get address of top page: fork_proc_ptr->p_page + 0x0f
        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        ; load p_page
        ld e, (hl)
        inc hl
        ld d, (hl)
        ; add 0x000F
        ld hl, #0x000F
        add hl, de
        ; load into MMU
        ld a, #0x0F
        out (MMU_SELECT), a
        ld a, h
        out (MMU_FRAMEHI), a
        ld a, l
        out (MMU_FRAMELO), a

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
        ld  hl, #0
        ret

