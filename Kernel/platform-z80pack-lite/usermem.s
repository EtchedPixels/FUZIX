;
;	We can't flip arbitary pairs of banks into memory so this one
;	is quite horrid. Defintiely worth optimising
;
        .module usermem

        .include "kernel.def"
        .include "../kernel.def"

        ; exported symbols
        .globl _uget
        .globl _ugetc
        .globl _ugets
        .globl _ugetw

        .globl _uput
        .globl _uputc
        .globl _uputw
        .globl _uzero

	.globl bankfork
;
;	We need these in common as we don't have flexible banks
;
        .area _COMMONMEM
bouncebuffer:
	.ds 256

uputget:
        ; load HL with the source address
        ld l, 4(ix) ; src address
        ld h, 5(ix)
        ; load DE with destination address (in userspace)
        ld e, 6(ix)
        ld d, 7(ix)
        ; load BC with the byte count
        ld c, 8(ix) ; byte count
        ld b, 9(ix)
	ld a, b
	or c
	ret


;
;	Copy a block of bytes, length in bc
;	HL = source, DE = dest, A = dest bank A' = source bank
;
;	On return BC trashed, HL = next source to fetch
;	DE = next dest to use
;
copyio:
	ex af,af
	out (21), a		; source bank
	ex af,af
	push de
	ld de, #bouncebuffer
	push bc
	ldir
	out (21), a
	pop bc
	pop de
	push hl
	ld hl, #bouncebuffer
	ldir
	pop hl
	ret	
	
copyblock:
	inc b
	dec b			; check if B is 0
	jr z, copytail		; just a tail
copyloop:
	push bc
	ld bc, #0x100		; do 256 byte blocks
	call copyio
	pop bc
	djnz copyloop
copytail:
	; on entry B will be 0 already, C will be the residue
	inc c
	dec c
	call nz, copyio
	xor a
	out (21), a
	ret


_uputc:
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
        ; load HL with destination address (in userspace)
	ld a, (U_DATA__U_PAGE)		; we use the first byte for our bank
	out (21), a			; stack gone for a walk
	ld (hl), e
uputc_out:
	xor a
	out (21), a		; map kernel back
	
	; Preserve HL as we use this path for ugetc/w returns
	pop af
	ret po
	ei
	ret

_uputw:
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
        ; load HL with destination address (in userspace)
	ld a, (U_DATA__U_PAGE)		; we use the first byte for our bank
	out (21), a			; stack gone for a walk
	ld (hl), e
	inc hl
	ld (hl), d
	jr uputc_out

_ugetc:
	pop bc	; return
	pop hl	; address
	push hl
	push bc
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
        ; load HL with source address (in userspace)
	ld a, (U_DATA__U_PAGE)		; we use the first byte for our bank
	out (21), a			; stack gone for a walk
        ld l, (hl)
	ld h, #0
        jr uputc_out

_ugetw:
	pop bc	; return
	pop hl	; address
	push hl
	push bc
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
        ; load HL with source address (in userspace)
	ld a, (U_DATA__U_PAGE)		; we use the first byte for our bank
	out (21), a			; stack gone for a walk
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
        jr uputc_out

_uget:
        push ix
        ; stack has: ix, return address, source, dest, count. 
        ;                                ix+4    ix+6  ix+8
        ld ix, #0   ; load ix with stack pointer
        add ix, sp
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
	call uputget
	jp z, uput_out
	ld a, (U_DATA__U_PAGE)	; A' is source (process)
	ex af, af
	xor a			; A is dest (kernel)
	call copyblock
	jr uput_out


_uput:
        push ix
        ; stack has: ix, return address, source, dest, count. 
        ;                                ix+4    ix+6  ix+8
        ld ix, #0   ; load ix with stack pointer
        add ix, sp
        ; store interrupt state, disable interrupts
	; FIXME: may not be reliable on an NMOS Z80 (review erratum)
        ld a, i
        di
        push af
	call uputget		; DE = dest, HL = src, BC = count
	jr z, uput_out

				; copy up to 256 bytes into the bounce
				; buffer
	xor a			; A' is source (kernel)
	ex af, af
	ld a, (U_DATA__U_PAGE)	; A is dest (process)
	call copyblock
uput_out:
	pop af
	pop ix
	ret po
	ei
	ret

;
;	Crap implementation for now
;
;	Given all the bounce buffers needed and the fact that we know
;	that
;	a) there is no MMIO to hit
;	b) the kernel side buffer is always big enough for worst
;	   case
;
;	FIXME:
;	We just block copy the lot. We ought to trim the length according
;	to userspace end first
;
_ugets:
        push ix
        ; stack has: ix, return address, source, dest, count. 
        ;                                ix+4    ix+6  ix+8
        ld ix, #0   ; load ix with stack pointer
        add ix, sp
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
	call uputget
	jr z, uput_out
	push de			; save pointer
	push bc			; save length
	ld a, (U_DATA__U_PAGE)	; A' is source (process)
	ex af, af
	xor a			; A is dest (kernel)
	call copyblock
	pop hl			; recover them but in HL
	pop bc
ugets_0:ld a, (hl)		; find an end marker
	or a
	jr z, ugets_good
	dec bc
	ld a, b
	or c
	jr nz, ugets_0
	dec hl
	ld (hl), #0
	ld hl, #0xffff			; flag as bad
	jr uput_out
ugets_good:
	ld hl, #0			; return 0
	jr uput_out
;
;	No bounce buffer needed as we are just filling the target
;
_uzero:
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl	
	push de
	ld a, b	; check for 0 copy
	or c
	ret z
        ; store interrupt state, disable interrupts
        ld a, i
        di
        push af
	ld a, (U_DATA__U_PAGE)		; we use the first byte for our bank
	out (21), a			; stack gone for a walk
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out


;
;	This is related so we will keep it here. Copy the process memory
;	for a fork. a is the page base of the parent, c of the child
; 	(this API will be insufficient once we have chmem and proper use of
; 	banks - as well as needing to support fork to disk)
;
;	Assumption - fits into a fixed number of whole 256 byte blocks
;
bankfork:
;	ld bc, #(0xC000 - 768)		;	48K minus the uarea stash

	ld b, #0xBD		; C0 x 256 minus 3 sets for the uarea stash
	ld hl, #0		; base of memory to fork (vectors included)
bankfork_1:
	push bc			; Save our counter and also child offset
	push hl
	out (21), a		; switch to parent bank
	ld de, #bouncebuffer
	ld bc, #256
	ldir			; copy into the bounce buffer
	pop de			; recover source of copy to bounce
				; as destination in new bank
	pop bc			; recover child port number
	push bc
	ld b, a			; save the parent bank id
	ld a, c			; switch to the child
	out (21), a
	push bc			; save the bank pointers
	ld hl, #bouncebuffer
	ld bc, #256
	ldir			; copy into the child
	pop bc			; recover the bank pointers
	ex de, hl		; destination is now source for next bank
	ld a, b			; parent back is wanted in a
	pop bc
	djnz bankfork_1		; rinse, repeat
	ret

