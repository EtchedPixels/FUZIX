;
;	MAP80 up to 256K paged in a different way to the page mode
;	and using port 0xFE. It is possible to combine the two systems
;	but that way lies madness.
;
;	We run it in 64K mode with no fixed blocks.
;

#include "../../build/kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

#ifdef CONFIG_MAP80

	; exported symbols
	.export init_hardware
	.export map_kernel
	.export map_kernel_di
	.export map_kernel_restore
	.export map_proc
	.export map_proc_di
	.export map_proc_a
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap
	.export map_buffers
	.export size_ram


	.common		; so we are upper 32K

;
;	Scan the 16 possible banks and put the mask into DE. In theory
;	you could fit 8 boards into a backplane and do 2MB in 64K mode only
;	but that was never realistically possible so we don't handle it.
;
size_ram:
	ld hl,0x0004		; An address we can use for testing
				; H must be 0
	ld de,0
	; Quick check if we have a map 80
	ld a,0xC0
	out (0xFE),a
	ld (hl),a		; Write a token
	ld a,0xC2
	out (0xFE),a
	ld (hl),a		; On a MAP80 this goes elsewhere
	ld a,0xC0
	out (0xFE),a		; Base page
	cp (hl)			; If it was scribbled on...
	ret nz			; No MAP80 present


	; A is already C0	; 32K mode, fixed upper, bank 0
	ld b,16			; 16 possible 64K banks
size_loop:
	rl e			; DE is the mask of pages present
	rl d

	out (0xFE),a		; Switch low 32K
	ld (hl),a		; Test address 4
	inc (hl)
	cp (hl)
	jr z, missing
	dec (hl)
	cp (hl)
	jr nz, missing
	set 0,e	
missing:
	inc a
	inc a			; Move on a 64K chunk
	djnz size_loop
	; DE now holds a bitmask of present pages. There can be up to 16
	; 64K pages split across 4 cards and they may be sparsely populated
	; in theory at least.
	xor a
	out (0xFE),a
	ret

init_hardware:
	call size_ram
	push de
	ld bc,#0xC3FE		; bank 1, 32K high fixed, map hig half, port FE
	rl e			; bank 0 is always present
	rl d
	ld a,1			; bank count
next_bank:
	bit 7,d
	jr z, empty_bank
copy_next:
	out (c),b		; map target bank low
	exx
	ld hl,#0xC000
	ld de,#0x4000
	ld b,d
	ld c,e
	ldir
	exx
	inc a
empty_bank:
	rl e
	rl d
	inc b
	inc b
	bit 5,b
	jr z, next_bank
	pop de

	ld (_bankmap),de	; Save the map for C

	ld b,0
	out (c),b		; Bank to normal banking

	; A lots of 64K (0-15)
	ld l,0
	srl a
	rr l
	srl a			; now in bits 5-2
	rr l
	ld h,a
	ld (_ramsize), hl
	ld de, #64		; for kernel
	or a
	sbc hl, de
	ld (_procmem), hl
	ld hl,0
	push hl
	call _program_vectors
	pop af
	; RST helpers
	ld hl,rstblock
	ld de,8
	ld bc,32
	ldir
	; FIXME: interrupt mode is per port target ?
	ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .commondata

pagereg:    .byte 0x00		; bank 0
pagesave:   .byte 0x00		; saved copy

	.common
;
;	Mapping set up for the MAP80 mode
;
;	The kernel is in bank 0, the user processes in the other banks. We
;	take care to keep common writables in COMMONDATA which is the area
;	of unbanked memory.
;
map_kernel:
map_kernel_di:
map_kernel_restore:
map_buffers:
	push af
	xor a
	ld (pagereg),a
	out (0xFE), a
	pop af
	ret
;
;	Do the page mode switch
;
map_proc:
map_proc_di:
	ld a, h
	or l
	jr z, map_kernel
	ld a, (hl)
map_for_swap:
map_proc_a:			; used by bankfork
	ld (pagereg),a
	out (0xFE),a
	ret

map_proc_always:
map_proc_always_di:
	push af
	push hl
	ld hl, #_udata + U_DATA__U_PAGE
	ld a, (hl)
	ld (pagereg),a
	out (0xFE),a
	pop hl
	pop af
	ret

map_save_kernel:
	push af
	ld a,(pagereg)
	ld (pagesave), a
	xor a
	ld (pagereg),a
	out (0xFE), a
	pop af
	ret

map_restore:
	push af
	ld a, (pagesave)
	ld (pagereg), a
	out (0xFE), a
	pop af
	ret

;	Stub helpers for code compactness. TODO for new compiler
;
	.discard

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
#endif
