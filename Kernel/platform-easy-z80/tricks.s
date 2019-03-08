        .module tricks

        .include "kernel.def"
        .include "../kernel-z80.def"

TOP_PORT	.equ	MPGSEL_3

	.include "../lib/z80bank16.s"

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
	ldir
	exx
	call map_kernel		; put the maps back so we can look in p_tab
	djnz fork_next
	ld a, c
	ld (mpgsel_cache+3),a	; cache the page number
	out (MPGSEL_3), a	; our last bank repeats up to common
	; --- we are now on the stack copy, parent stack is locked away ---
	ret			; this stack is copied so safe to return on

	
