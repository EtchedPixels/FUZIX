;
;	NASCOM/Gemini 'page mode' banking
;
#include "../../build/kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

#ifndef CONFIG_MAP80

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

;
;	We keep the probe routine in the common copy area as we need
;	to bank flip as we probe
;
	.common
banktest:
	out (0xFF),a
	ld a,0xAA
	cp (hl)
	ret nz
	inc (hl)
	ld (de),a		; any old load, just cause bus traffic
	inc a
	cp (hl)
	ret

size_ram:
	ld hl,0x0004
	ld (hl),0			; Write current bank (1) with 0
	ld a,0x21			; Write 2 read 1
	out (0xFF),a
	ld (hl),a			; Write to bank 2 we hope
	cp (hl)			; Did write go to reading bank
	jr nz, has_bank		; If it did we get NZ and skip
	ld hl,64			; No paged memory, just base
	ld a,0x11
	out (0xFF),a		; Back to memory normality
	ret
has_bank:
	ld de,0x0005
	ld a,0xF1
	out (0xFF),a		; write all
	; Now test each bank and see who is present. There can be holes
	ld a,0xAA
	ld (hl),a
	xor a
	ld c,a
	ld b,a
	ld (de),a
	ld a,0x11
size_loop:
	push af
	call banktest
	jr nz,nobank
	set 0,c		; mask of banks present
	inc b		; count of banks present
nobank:
	pop af
	sla c
	add a,a
	jr nc,size_loop
	ld a,0x11
	out (0xFF),a	; back to kernel map
	srl c
	ld (_bankmap),bc	; count and mask
	ld a,b		; number of banks present (including base)
	add a,a
	add a,a
	add a,a
	add a,a		; x 16
	ld h,0
	ld l,a
	ld d,h
	ld e,a
	add hl,hl		; x 32
	add hl,de 		; x 48
	ld de,16		; base memory
	add hl,de
	ret

	.code

init_hardware:
	ld a,0xF1		; Write all banks read kernel
	out (0xFF),a
	ld hl, #PROGBASE	; Program base is paged space base
	ld d,h
	ld e,l
	ld bc, #0xF000	; non CPM 1000-FFFF (but the top is ROM so no
				; problem)
				; CP/M 0000-EFFF so good
	ldir		; Duplicate everything (it's simplest)
	ld a,0x11
	out (0xFF),a
	; Only now is it safe to call into common space
	call size_ram
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

pagereg:    .byte 0x11		; bank 1 R/W
pagesave:   .byte 0x11		; saved copy

	.common
;
;	Mapping set up for the Nascom 'Page mode'
;
;	The kernel is in bank 1, the user processes in the other banks. We
;	take care to keep common writables in COMMONDATA which is the area
;	of unbanked memory.
;
map_kernel:
map_kernel_di:
map_kernel_restore:
map_buffers:
	push af
	ld a,0x11
	ld (pagereg),a
	out (0xFF), a
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
	out (0xFF),a
	ret

map_proc_always:
map_proc_always_di:
	push af
	push hl
	ld hl, #_udata + U_DATA__U_PAGE
	ld a, (hl)
	ld (pagereg),a
	out (0xFF),a
	pop hl
	pop af
	ret

map_save_kernel:
	push af
	ld a,(pagereg)
	ld (pagesave), a
	ld a,0x11
	ld (pagereg),a
	out (0xFF), a
	pop af
	ret

map_restore:
	push af
	ld a, (pagesave)
	ld (pagereg), a
	out (0xFF), a
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
