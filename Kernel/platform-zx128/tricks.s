; Based on the Z80 Pack banked code
;
; Should unify this somewhat with lib/banked
;

        .module tricks

        .globl _ptab_alloc
        .globl _newproc
        .globl _getproc
        .globl _plt_monitor
        .globl _plt_switchout
        .globl _switchin
	.globl _low_bank
	.globl _dup_low_page
        .globl _dofork
        .globl _runticks
        .globl unix_syscall_entry
        .globl interrupt_handler
	.globl current_map
	.globl _ptab
	.globl _swapper

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

	;
	; We are now running on the sleeping process stack. The switchin
	; will simply go back to the saved SP above and discard anything
	; here
	;

	; Stash the uarea back into process memory
	ld hl, (_udata + U_DATA__U_PAGE)
	ld a, l
	ld bc, #0x7ffd
	or #0x18
	out (c), a

	; This includes the stacks, so be careful on restore
	ld hl, #_udata
	ld de, #U_DATA_STASH
	ld bc, #U_DATA__TOTALSIZE
	ldir
	ld a, (current_map)
	or #0x18
	ld bc, #0x7ffd
	out (c), a

        ; find another process to run (may select this one again)
	push af
        call _getproc
	pop af		; we can't optimise this as the linker
			; is entitled to patch the 5 bytes here into a
			; banked call
        push hl
	push af
        call _switchin

        ; we should never get here
        call _plt_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0

;
;	FIXME: update this to use both pages and do the needed exchange
;	on low pages.
;
;	FIXME: need to add swap to this yet 8(
;
_switchin:
        di
	pop hl	; far padding
        pop bc  ; return address
        pop de  ; new process pointer
;
;	FIXME: do we actually *need* to restore the stack !
;
        push de ; restore stack
        push bc ; restore stack
	push hl ; far padding

        ld hl, #P_TAB__P_PAGE_OFFSET
	add hl, de	; process ptr
	;
	;	Get ourselves a valid private stack ASAP. We are going to
	;	copy udata around and our main stacks are in udata
	;
	ld sp, #_swapstack

        ld a, (hl)	; 0 swapped, not zero is the bank for C000
	or a
	jr nz, not_swapped

	; Swap the process in (this may swap something else out first)
	; The second pushes are C function arguments. SDCC can trample
	; these

	push hl		; Save
	push de
	push hl		; Arguments
	push de
	push af
	call _swapper
	pop af
	pop af
	pop af
	pop de		; Restore
	pop hl
	ld a, (hl)	; We should now have a page assigned
not_swapped:
	; We are in DI so we can poke these directly but must not invoke
	; any code outside of common
	or #0x18	; ROM
	; Pages please !
	ld bc, #0x7ffd
	out (c), a

	;	Copy the stash from the user page back down into common
	;	The alternate registers are free - we use them for the
	;	block copy and for the flipper
	;
	;	FIXME: Add Tormod's optimisation from the 6809 tree
	;
	exx
	ld hl, #U_DATA_STASH
	ld de, #_udata
	ld bc, #U_DATA__TOTALSIZE
	ldir
	exx

	;
	;	 Remap the kernel proper
	;

	ld a, (current_map)
	or #0x18
	ld bc, #0x7ffd
	out (c), a
        
	;	Is our low data in 0x8000 already or do we need to flip
	;	it with bank 6. _low_bank holds the page pointer of the
	;	task owning the space

	ld hl, (_low_bank)	; who owns low memory
	or a
	sbc hl, de		; preserve DE
	jr z, nofliplow		; skip the flip if we own low bank
;
;	Flip low banks over. Interrupts must be off as the low banks don't
;	have the IM2 vectors. Preserve DE
;
fliplow:
	exx
	ld hl, #0x8000
	ld de, #0xc000
	ld a, #6 + 0x18
	ld bc, #0x7ffd
	out (c), a
flip2:
	ld b, #0		; 256 bytes per outer loop (16K total)
flip1:
	ld c, (hl)
	ld a, (de)
	ex de, hl
	ld (hl), c
	ld (de), a
	inc hl
	inc de
	djnz flip1
	xor a
	cp d			; Wrapped to 0x0000 ?
	jr nz, flip1

	ld a, (current_map)
	or #0x18
	ld bc, #0x7ffd
	out (c), a
	exx
	ld (_low_bank), de	; we own it now

nofliplow:

        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE == HL
        jr nz, switchinfail

	; wants optimising up a bit
	ld ix, (_udata + U_DATA__U_PTAB)
        ; next_process->p_status = P_RUNNING
        ld P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING

	; Fix any moved page pointers
	; Just do one byte as that is all we use on this platform
	ld a, P_TAB__P_PAGE_OFFSET(ix)
	ld (_udata + U_DATA__U_PAGE), a
        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (_udata + U_DATA__U_SP)

	;
	; We can now use the stack again
	;

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

; Interrupts should be off when this is called
_dup_low_page:
	ld a, #0x06 + 0x18		;	low page alternate
	ld bc, #0x7ffd
	out (c), a

	ld hl, #0x8000			; 	Fixed
	ld de, #0xC000			;	Page we just mapped in
	ld bc, #16384
	ldir

	ld a, (current_map)		; 	restore mapping
	or #0x18			; ROM bits
	ld bc, #0x7ffd
	out (c), a
	ret

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

	; Need to write a new 16K bank copy here, then copy the live uarea
	; into the stash of the new process

        ; --------- copy process ---------

        ld hl, (fork_proc_ptr)
	ld (_low_bank), hl		;	low bank will become the child
        ld de, #P_TAB__P_PAGE_OFFSET	;	bank number
        add hl, de
        ; load p_page
        ld c, (hl)
	ld hl, (_udata + U_DATA__U_PAGE)
	ld a, l

	;
	; Copy the high page via the bounce buffer
	;

	call bankfork			;	do the bank to bank copy

	; FIXME: if we support small apps at C000-FBFF we need to tweak this
	; Now copy the 0x8000-0xBFFF area directly

	ld a, #0x06 + 0x18		;	low page alternate
	ld bc, #0x7ffd
	out (c), a

	ld hl, #0x8000			; 	Fixed
	ld de, #0xC000			;	Page we just mapped in
	ld bc, #16384
	ldir

	; Copy done

	ld a, (_udata + U_DATA__U_PAGE)	; parent memory
	or #0x18		; get the right ROMs
	ld bc, #0x7ffd
	out (c), a		; Switch context to parent in 0xC000+

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
	ld bc, #0x7ffd
	ld a, (current_map)
	or #0x18
	out (c), a
        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.

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
	or #0x18		; ROM bits for the bank
	ld b, #0x3C		; 40 x 256 minus 4 sets for the uarea stash/irqs
	ld hl, #0xC000		; base of memory to fork (vectors included)
bankfork_1:
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
	or #0x18		; ROM bits
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
_low_bank:
	.dw _ptab			; Init starts owning this

