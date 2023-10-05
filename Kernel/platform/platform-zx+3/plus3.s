;
;    Spectrum +3 support

        .module plus3

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler
	.globl nmi_handler

        .globl map_kernel
        .globl map_proc_always
        .globl map_proc
        .globl map_kernel_di
        .globl map_proc_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_kernel_restore
	.globl map_for_swap
	.globl map_video
	.globl current_map

        .globl _need_resched
	.globl _int_disabled
	.globl _vtborder
	.globl diskmotor

        ; exported debugging tools
        .globl _plt_monitor
	.globl _plt_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

	.globl _vtoutput
	.globl _vtinit

        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex

	.globl ___sdcc_enter_ix

        .include "kernel.def"
        .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (above 0xF000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	;
	;	Not so much a monitor as wait for space
	;
	ld bc,#0x1ffd
	ld a,#0x01
	out (c),a		; keep us mapped, turn off motors
	ld a, #0x7F
	in a, (0xFE)
	rra
	jr c, _plt_monitor

_plt_reboot:
	di
	ld bc,#0x7ffd
	ld a,#0x03
	out (c),a	; set 128K paging to put page 3 at the top (ie us)
			; and ROM in
	ld bc,#0x1ffd
	xor a		; flip to normal paging 128K ROM motor off
	out (c),a
        rst 0		; back into our booter

plt_interrupt_all:
        ret

	.area _COMMONMEM

_int_disabled:
	.db 1

_vtborder:		; needs to be common
	.db 0


; -----------------------------------------------------------------------------
; KERNEL CODE BANK (below 0xC000)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:
	call _program_early_vectors
        ret

init_hardware:
        ; set system RAM size
        ld hl, #128
        ld (_ramsize), hl
        ld hl, #64	      ; 64K for kernel/screen/etc (FIXME)
        ld (_procmem), hl

	; Install rst shorteners
	ld hl,#rstblock
	ld de,#8
	ld bc,#32
	ldir

	ld bc,#0x7ffd
	ld a,#0x0B		; bank 3 (common) in high in either mapping
				; video bank 7
	out (c),a		; and we should have special mapping
				; already by now	
	xor a
	out (0xFE),a		; black border
        ; screen initialization
	call _vtinit

        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_early_vectors:
	call map_proc_always
	call set_vectors
	call map_kernel
set_vectors:
        ; write zeroes across all vectors
        ld hl, #0
        ld de, #1
        ld bc, #0x007f ; program first 0x80 bytes only
        ld (hl), #0x00
        ldir

        ; now install the interrupt vector at 0x0038
        ld a, #0xC3 ; JP instruction
        ld (0x0038), a
        ld hl, #interrupt_handler
        ld (0x0039), hl

        ; set restart vector for FUZIX system calls
        ld (0x0030), a   ;  (rst 30h is unix function call vector)
        ld hl, #unix_syscall_entry
        ld (0x0031), hl

        ld (0x0000), a   
        ld hl, #null_handler   ;   to Our Trap Handler
        ld (0x0001), hl

        ld (0x0066), a  ; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl

_program_vectors:
	ret

	; Swap helper. Map the page in A into the address space such
	; that swap_map() gave the correct pointer to use. Undone by
	; a map_kernel_{restore}
map_proc:
        ld a, h
        or l
        jr z, map_kernel
map_for_swap:
map_proc_always:
map_proc_always_di:
	push af
	ld a,#0x1			; 0 1 2 3
	jr map_a_pop
;
;	Save and switch to kernel
;
map_save_kernel:
	push af
        ld a, (current_map)
        ld (map_store), a
	pop af
map_kernel_di:
map_kernel:
map_kernel_restore:
	push af
	ld a,#0x05			; 4 5 6 3
map_a_pop:
	push bc
	ld (current_map),a
	ld bc,(diskmotor)
	or c
	ld bc,#0x1ffd
	out (c),a
	pop bc
	pop af
	ret

map_video:
	push af
	ld a,#0x07			; 4 7 6 3
	jr map_a_pop

map_restore:
	push af
        ld a, (map_store)
	jr map_a_pop

;
;	We have no easy serial debug output instead just breakpoint this
;	address when debugging.
;
outchar:
	ld (_tmpout), a
	push bc
	push de
	push hl
	push ix
	ld hl, #1
	push hl
	ld hl, #_tmpout
	push hl
	call _vtoutput
	pop af
	pop af
	pop ix
	pop hl
	pop de
	pop bc
        ret

	.area _COMMONMEM
_tmpout:
	.db 1

current_map:                ; place to store current page number. Is needed
        .db 0               ; because we have no ability to read 0xF4 port
                            ; to detect what page is mapped currently 
map_store:
        .db 0

_need_resched:
        .db 0

diskmotor:
	.db 0

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
