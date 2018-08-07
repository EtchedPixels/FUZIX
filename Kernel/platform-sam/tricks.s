
	.include "../kernel.def"
	.include "kernel.def"

;FIXME	.include "../lib/z80fixedbank.s"

	.module tricks32

	.globl _ptab_alloc
	.globl _newproc
	.globl _chksigs
	.globl _getproc
	.globl _runticks
	.globl _platform_monitor

	.globl _platform_switchout
	.globl _switchin
	.globl _dofork

	.globl map_page_low
	.globl map_kernel_low

	.globl outhl
	.globl outstring

	.globl _platform_copier_l
	.globl _platform_copier_h

	.area _HIGH

;
;	Switch out this process for another task. All the optimisation
;	tricks have been done by switchout, so when we are called we
;	really need to switch.
;
_platform_switchout:
	di
	ld hl,#0
	push hl		; stack a retcode of 0 (see fork)
	push ix		; save registers
	push iy
	ld (U_DATA__U_SP), sp

	; Map the to
	ld a,(U_DATA__U_PAGE2)
	call map_page_low
	ld hl,#U_DATA
	ld de,#U_DATA_STASH-0x8000
	ld bc,#U_DATA__TOTALSIZE
	ldir
	call map_kernel_low
	; switch the new process in
	call _getproc
	push hl
	call _switchin
	; Should never be hit
	call _platform_monitor

_switchin:
	di
	pop bc	; return address (we never do)
	pop de	; new process pointer

	ld hl,#P_TAB__P_PAGE2_OFFSET
	add hl,de
	ld a,(hl)	; our high page

	; FIXME: add swap support

	ld hl,(U_DATA__U_PTAB)
	or a
	sbc hl,de
	jr z, skip_copyback

	call map_page_low

	; We are going to write our stack directly under us. No calls for
	; a moment
	exx
	ld hl,#U_DATA_STASH
	ld de,#U_DATA
	ld bc,#U_DATA__TOTALSIZE
	ldir
	exx
	ld sp, (U_DATA__U_SP)

	; Ok our stack is now valid

	call map_kernel_low
	ld hl, (U_DATA__U_PTAB)
	or a
	sbc hl,de
	jr nz, switchin_failed	; invalid u_data

skip_copyback:
	ld sp, (U_DATA__U_SP)
	ld ix, (U_DATA__U_PTAB)
	ld P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING
	ld a, P_TAB__P_PAGE_OFFSET(ix)
	ld (U_DATA__U_PAGE),a
	ld a, P_TAB__P_PAGE2_OFFSET(ix)
	ld (U_DATA__U_PAGE2),a
	ld hl,#0
	ld (_runticks), hl
	; Recover IX and IY, return value
	pop ix
	pop iy
	pop hl

	; if we pre-empted in an ISR IRQ's stay off, if not they get enabled
	ld a, (U_DATA__U_ININTERRUPT)
	or a
	ret nz
	ei
	ret

switchin_failed:
	call outhl
	ld hl, #bad_switch
	call outstring
	jp _platform_monitor

fork_proc_ptr:
	.dw 0

bad_switch:
	.asciz '!SWITCH'

_dofork:
	di
	pop de
	pop hl		; HL is the new process ptab pointer
	push hl
	push de

	ld (fork_proc_ptr),hl

	ld de,#P_TAB__P_PID_OFFSET
	add hl,de
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a

	; Build a parent frame for switchin that has the child pid
	; on it
	push hl
	push ix
	push iy

	ld (U_DATA__U_SP), sp

	; Parent state built and in u_data

	call copy_process

	; Low page is undefined at this point which is ok as we are
	; about to change it again

	; Now put the u_data into the stash for the parent, then we can
	; modify it for the child

	; Map parent
	ld a,(U_DATA__U_PAGE)
	call map_page_low
	; Copy udata
	ld hl, #U_DATA
	ld de, #U_DATA_STASH - 0x8000
	ld bc, #U_DATA__TOTALSIZE
	ldir
	; Kernel back
	call map_kernel_low	

	pop bc			; discard stack frame we built
	pop bc
	pop bc

	; Back into the kernel to finish the process creation

	ld hl,(fork_proc_ptr)
	push hl
	call _newproc
	pop bc

	; Clear runticks, and return 0
	ld hl,#0
	ld (_runticks),hl
	ret

;
;	Process copying is tricky. We want to do a direct copy for speed
;	but that requires mapping both banks and means
;	1. We can't put all of this routine in a common helper when we make one
;	2. We need interrupts off for now
;	3. We need a magic helper in the low page
;
;	HL is the child ptab, udata is the parent
;
;	The platform needs to provide a platform_copier method for each
;	bank that copies low to high of all process, stubs and user data
;	in that bank (basically a straight 32K copy)
;
;	typically that would be
;
;	ld a,(hl)
;	out (whatever),a
;	exx
;	ldir
;	exx
;	ld a,#kernel
;	out (whatever),a
;	ret
;
;	There have to be two of these - one in high stubs one in low because
;	the high banks will both be mapped so the low one isn't available
;
copy_process:
	ld de,#P_TAB__P_PAGE2_OFFSET
	add hl,de
	ld a,(U_DATA__U_PAGE)
	call map_page_low
	call setup_platform_copier
	call _platform_copier_l
	inc hl
	inc hl
	ld a,(U_DATA__U_PAGE2)
	call map_page_low
	call setup_platform_copier
	call _platform_copier_h
	ret

setup_platform_copier:
	exx
	ld hl,#0
	ld de,#0x8000
	ld b,d
	ld c,e
	exx
	ret