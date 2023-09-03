;
;	Fast user memory copiers for RC2014
;
;	These rely upon the fact that 0x4000-0xBFFF contain only code so
;	from the kernel's point of view are never eligible locations
;	to copy to or from. That in turn means that we can use them as a
;	32K window to safely do any copy up to 16K in size to or from
;	user space with LDIR or DMA.
;
;	For now don't use the DMA hooks with uput/uget. This needs more
;	instrumentation to check but it appears most copies are small
;	because aligned block sized I/O (most of it) goes direct.
;

	.module rc2014usermem


	.include "kernel.def"
        .include "../kernel-z80.def"

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl _udata

	.globl  map_proc_save
	.globl  map_kernel_restore

	.globl ldir_or_dma

	.globl mpgsel_cache
;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

;
;	Zero a chunk of memory.
;
__uzero:
	pop iy
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl
	push de
	push iy
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_proc_save
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	; This won't adjust HL/DE/BC as LDIR does in all cases
	; but that isn't a problem.
	call ldir_or_dma
	jr uputc_out

;
;	We can't really optimize these much
;
__uputc:
	pop iy	;	bank
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_save
	ld (hl), e
uputc_out:
	ld hl,#0
	jp map_kernel_restore			; map the kernel back below common

__uputw:
	pop iy
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_save
	ld (hl), e
	inc hl
	ld (hl), d
	jr uputc_out

__ugetc:
	call map_proc_save
        ld l, (hl)
	ld h, #0
	jp map_kernel_restore

__ugetw:
	call map_proc_save
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel_restore


;
;	General helper to get the C arguments off the stack. Not entirely
;	pretty on a Z80.
;
uputget:
        ; load BC with the byte count
        ld c, 10(ix) ; byte count
        ld b, 11(ix)
        ; load HL with the source address
        ld l, 6(ix) ; src address
        ld h, 7(ix)
        ; load DE with destination address (in userspace)
        ld e, 8(ix)
        ld d, 9(ix)
	ld a, b
	or c
	ret

;
;	Make the user pages holding DE upwards appear at 0x4000-0xBFFF
;	Return an updated DE with the mapped address. User A and alt
;	registers. Take care on the mappings as we could be interrupted
;	so must maintain mpgsel_cache properly.
;
user_map_de:
	ld a,d				; user address
	exx
	rlca
	rlca
	and #3
	ld e,a
	ld d,#0
	ld hl,#_udata + U_DATA__U_PAGE
	add hl,de
	ld a,(hl)			; Page holding low 16K of our map
	ld (mpgsel_cache + 1),a
	out (MPGSEL_1),a
	inc hl
	ld a,(hl)
	ld (mpgsel_cache + 2),a
	out (MPGSEL_2),a
	exx

	; 4000-BFFF is now a user mapping

	ld a,d
	and #0x3F			; get non page bits
	add #0x40			; add page base 0x40
	ld d,a
	ret

;
;	Copy data to user space
;
__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC

	; Check if we are done
uput_next:
	jr z, uput_out			; but count is at this point magic

	; We can't do more than 16K in one go because that might fall out
	; of our mapped range
	ld a,b
	and #0xC0
	jr nz, uput_large		; special case large transfers

	; Map user into 0x4000-0xBFFF and then copy
	call user_map_de
copy_and_out:
	ldir
	call map_kernel_restore
uput_out:
	pop ix
	ld hl, #0
	ret

uput_large:
	push bc
	push de
	call user_map_de
	ld bc,#0x4000			; 16K chunks
	ldir				; Updates HL
	pop bc				; Recover count
	pop de				; Recover use space DE for next go
	ld a,d				; Move on 16K
	add #0x40
	ld d,a
	ld a,b				; Drop count 16K
	sub #0x40
	ld b,a
	or c				; Check for exact 16K sizes
	jr uput_next			; See if we are doing 16K or not

;
;	The same logic as uput but we copy the other direction
;
__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
uget_next:	
	jr z, uput_out			; but count is at this point magic

	ld a,b
	and #0xC0
	jr nz, uget_large		; special case large transfers

	ex de,hl			; want to make the source
	call user_map_de
	ex de,hl
	jr copy_and_out

;
;	BC is the size (>= 0x4000)
;	DE is the kernel address we copy to and update
;	HL is the user address
;
uget_large:
	push bc
	push hl
	ex de,hl
	call user_map_de
	ex de,hl
	ld bc,#0x4000			; 16K chunks
	ldir				; Updates DE
	pop bc				; Recover count
	pop hl				; Recover use space HL for next go
	ld a,h				; Move on 16K
	add #0x40
	ld h,a
	ld a,b				; Drop count 16K
	sub #0x40
	ld b,a
	or c				; Check for exactly 16K corner case
	jr uget_next			; See if we are doing 16K or not

