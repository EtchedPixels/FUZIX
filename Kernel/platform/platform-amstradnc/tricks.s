        .module tricks
	.globl _udata

        .include "platform/kernel.def"
        .include "../../kernel-z80.def" ; Kernel

TOP_PORT	.equ	0x13

	.include "../../lib/z80bank16.s"

        .area _COMMONMEM


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
	out (0x11), a		; 0x4000 map the child
	ld c, a
	inc hl
	ld a, (de)
	out (0x12), a		; 0x8000 maps the parent
	inc de
	cp #0x43		; graphics page  - same in both so skip
	jr z, skip_map
	exx
	ld hl, #0x8000		; copy the bank
	ld de, #0x4000
	ld bc, #0x4000		; we copy the whole bank, we could optimise
				; further
	ldir
	exx
skip_map:
	call map_kernel		; put the maps back so we can look in p_tab
	djnz fork_next
	ld a, c
	out (0x13), a		; our last bank repeats up to common
	; --- we are now on the stack copy, parent stack is locked away ---
	ret			; this stack is copied so safe to return on

	
