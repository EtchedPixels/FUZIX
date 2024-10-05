;
;	The MSX1 slot system is demented.
;

	.module bank_msx1

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

        ; imported symbols
        .globl _ramsize
        .globl _procmem
	.globl ___hard_di
	.globl outcharhex

;
;	Our helpers put the top bank back before returning so the rest
;	can be generalized in common space
;
	.area _COMMONMEM

	.globl _switch_map
	.globl _current_map
	.globl map_kernel
	.globl map_kernel_di
	.globl map_proc_always
	.globl map_proc_always_di
	.globl map_proc
	.globl map_save_kernel
	.globl map_restore
	.globl _set_initial_map
	.globl _map_slot1_kernel
	.globl _map_slot1_user
	.globl find_ram
	.globl _ramtab
	.globl _devtab
	.globl _kernel_map
	.globl _user_map
	.globl _current_map

	.globl _subslots
	.globl _int_disabled

	.globl do_set_sub_slot
	.globl do_get_sub_slot

;
;	Memory banking for a 'simple' MSX 1 system. We have Fuzix in a
;	cartridge from 0000-BFFF and RAM at C000-FFFF. For user space we
;	map the RAM from 0000-BFFF as well.
;
;	Isolate the banking logic so that we can try and share code with
;	other non MSX2 style mappers. (For an MSX2 mapper it would I think
;	be better to teach the MSX2 code about different VDP options).
;
;	Call our little helper who lives low down. It expects interrupts to be
;	*off*. The stack will be in the top 16K so care is neeed.
;
;	Entry
;	B = computed slot mask
;	A = subslot mask to set
;
;	Return
;	A = result of subslot set
;
;

set_sub_slot:
	push bc
	push de
	push hl
	ld c,#0xA8
	ld hl,#0xffff
	call do_set_sub_slot
	cpl
	pop hl
	pop de
	pop bc
	ret

get_sub_slot:
	push bc
	push de
	push hl
	ld c,#0xA8
	ld hl,#0xffff
	call do_get_sub_slot
	cpl
	pop hl
	pop de
	pop bc
	ret



;
;	Switch to the map pointed to by HL.
;
;	For our use case it's assumed the builder of the tables stuffs the
;	existing values in anything unchanged and for no subslot we always
;	use 0xFF.
;
;	HL points to the map
;	0: bitmask of slots that are expanded and within the selection
;		(within the selection is an optimization only)
;	1-4: subslot masks for slots 0-3
;	5: slot selection
;
;	DE points to the current map table
;
;	B is used to cycle the bits for A8 (00xxxxxx 01xxxxxx 10xxxxxx
;	11xxxxx)
;
;	On return the map change is done and the 
;
;	We try to optimize on the basis that most machines have less dumb
;	mappings than the worst case but this is still heavy by the
;	standards of the usual and rather less brain dead designs.
;
_switch_map:
	push af
	push bc
	push de
	ld a,#'['
;	out (0x2F),a
	in a,(0xA8)
	and #0x3F			;	Keep the lower selections
	ld b,a				;	the same
	ld c,(hl)
	inc hl				;	Points to subslot mask for slot 0
	ld de,#_current_map+1

subslot_next:
	bit 0,c
	jr z, subslot_done		;	no subslots
	ld a,(de)
	cp (hl)
	jr z, subslot_done		;	same subslots
;	ld a,b
;	call outcharhex
	ld a,(hl)			;	subslots for this bank
;	call outcharhex
	; Do set
	call set_sub_slot		;	Set the subslot
	ld (de),a			;	update map
subslot_done:
	inc hl				;	Next map
	inc de
	rr c
	;
	;	B is the mask of the slot code. Move on a slot in
	;	the top 16K
	;
	ld a,#0x40
	add b
	ld b,a
	jr nc, subslot_next
	; And flip the slot register
	ld a,(hl)
	ld (de),a
	out (0xA8),a
	ld a,#']'
;	out (0x2F),a
	pop de
	pop bc
	pop af
	ret

_current_map:
	.ds 6
_save_map:
	.ds 6
_kernel_map:
	.ds 6
_user_map:
	.ds 6
map_id:
	.db 0		; so we can fast figure out if returning to kernel/user
_scratch_map:
	.ds 6
_subslots:
	.db 0

	.area _CODE

;
;	Set up the initial mapping table. This is slightly pessimal as we
;	don't eliminate any unneeded loads in the resulting map. However we
;	want that map to be a reference everything uses so it needs to be
;	complete. The C code can optimize it later as it wishes.
;
;	Needs to be called before we blow away all the BIOS stuff.
;
_set_initial_map:
	ld hl,#0xFCC4
	ld b,#4
	xor a
subslot_mask:
	add a
	bit 7,(hl)
	dec hl
	jr z,not_exp
	inc a		; A was even before so this is safe
not_exp:
	djnz subslot_mask

	ld (_subslots),a

	ld hl, #0xFCC5
	ld de, #_kernel_map
	ld (de),a
	inc de
;
;	Now deal with the subslots. We can't quite assume the ROM is
;	correct because we updated some of it directly. Start by
;	copying the map. We will fix this up in a bit
;
	ldi
	ldi
	ldi
	ldi
	in a,(0xA8)
	ld (de),a
	and #0x03		; What bank is the ROM living in ?
	ld c,a
	ld b,#0
	ld hl,#_kernel_map+1
	add hl,bc		; subslot pointer for the kernel cartridge
				; or could be invalid if not subslotted but
				; we don't need to care
	ld a,(hl)
	and #0x0C		; propogate bits 2-3 across the low 6 bits
	rrca
	rrca
	ld c,a
	rlca
	rlca
	or c
	rlca
	rlca 
	or c
	ld c,a
	ld a,(hl)
	and #0xc0
	or c
	ld (hl),a
	
	; Set up memory information
	ld hl, #64
	ld (_ramsize),hl
	ld l, #48
	ld (_procmem),hl
	; Set the live table to agree with the hardware state
	ld hl, #_kernel_map
	ld de, #_current_map
	ldi
	ldi
	ldi
	ldi
	ldi
	ldi
	xor a
	ld (map_id),a
	ret

	.area _COMMONMEM

; Always called under interrupt disable and we never try and restore
; a user map. Just one of the kernel variants.
map_restore:
	push hl
	ld hl,#_save_map
	call _switch_map
	pop hl
	ret

; Always called under interrupt disable
map_save_kernel:
	push bc
	push de
	push hl
	ld de, #_save_map
	ld hl, #_current_map
	ldi
	ldi
	ldi
	ldi
	ldi
	ldi
	pop hl
	pop de
	pop bc
        call map_kernel_di		; FIXME: fast path this later
	ret

;
; The expensive bank switching also means we need to write some custom
; usermem copiers.
;
; FIXME: use map_id to eliminate bogus k/k switches (need to fix up the
; mmio and other cases before we can do this).
;
map_kernel:
	push af
	push hl
	di
	ld a,(_int_disabled)
	push af
	xor a
	ld (map_id),a
	; We are possibly coming from a user bank. We need to jam the
	; kernel low mapping back. FIXME: if the kernel cartridge and
	; RAM are in the same subslot this will break. Need to put the flip
	; code in the low 256 bytes of both.
	ld a,(_kernel_map+5)
	out (0xA8),a
	ld hl,#_kernel_map
switch_pop_out:
	call _switch_map
	pop af
	or a
	jr nz, was_di
	ei
was_di:
	pop hl
	pop af
	ret

map_kernel_di:
	push af
	push hl
	xor a
	ld (map_id),a
	; We are possibly coming from a user bank. We need to jam the
	; kernel low mapping back. FIXME: if the kernel cartridge and
	; RAM are in the same subslot this will break. Need to put the flip
	; code in the low 256 bytes of both.
	; (DONE OK now I think)
	ld a,(_kernel_map+5)
	out (0xA8),a
	ld hl,#_kernel_map
	call _switch_map
	pop hl
	pop af
	ret

map_proc:
	ld a,h
	or l
	jr z, map_kernel
	; Fall through
map_proc_always:
	push af
	push hl
	di
	ld a,(_int_disabled)
	push af
	ld a,#1
	ld (map_id),a
	ld hl,#_user_map
	jr switch_pop_out

map_proc_always_di:
	push af
	push hl
	ld a,#1
	ld (map_id),a
	ld hl,#_user_map
	call _switch_map
	pop hl
	pop af
	ret

;
;	Take a kernel or user mapping and prepare a temporary mapping
;	table that maps a device into 0x4000-0x7FFF. Device drivers can then
;	use this to switch mapping
;
;	Called with L = slot info
;
_map_slot1_kernel:
	push hl
	ld hl,#_kernel_map
	jr map_slot_1
_map_slot1_user:
	push hl
	ld hl,#_user_map
map_slot_1:
	ld de,#_scratch_map		; Copy the user or kernel map
	ldi
	ldi
	ldi
	ldi
	ldi
	ldi
	pop hl
	ld a,l
	; First step: Update the slot register in the map
	and #0x03			; slot
	rlca				; into the right bit positions
	rlca				; for 40-7F
	ld e,a
	ld a, (_scratch_map + 5)	; slot register
	and #0xF3			; mask out 40-7F
	or e				; add device bank identifier
	ld (_scratch_map + 5),a		; slot register with us in bank 1
	ld e,a				; Save it in E
	ld a,l				; Do we have subslots ?
	bit 7,a
	jr z, no_subslot_map
	; Second step: Update the subslot map for the slot in question
	; for addresses 0x4000-0x7FFF
	and #0x0C			; sub slot
	ld c,a				; save it
	ld de,#_scratch_map+1
	;  l already holds the slot we want to use
	ld h,#0
	add hl,de
	ld a,(hl)
	and #0xF3
	or c				; new subslot value
	ld (hl),a			; update map
no_subslot_map:
	; Return a pointer to the temporary map table
	ld hl,#_scratch_map
	ret



;
;	Find the RAM. Another fine MSX mess. Cycle through each slot
;	mapping it between 0x0000 and 0xBFFF then look for a ROM header
;	and if not try poking it.
;
;	We end up with a table of slot/subslot for each of the low 3 16K
;	banks. We don't worry about duplicates - MSX can't really do
;	anything with them. We could so with a different build I guess but
;	while in theory MSX machines can have 64K RAM shoved into multiple
;	subslots it's not normal and the 'big' memory cartridges all add an
;	MSX2 type mapper so want dealing with via an improved platform-msx2.
;
find_ram:
	ld ix,#0xFCC1
	ld iy,#_devtab		; device scan table
	ld b,#4
	ld e,#0
next_slot:
	push bc

	; Put the slot count into the low 6 bits
	ld a,e			; slot count
	rlca
	rlca
	or e
	rlca
	rlca
	or e			; lower bits
	ld d,a
	in a,(0xA8)
	push af			; save the old value
	and #0xC0
	or d			; A is now common with the low 48K switched

	bit 7,(ix)
	jr nz, sub_scan

	out (0xA8),a
	;
	;	No subslots - so just scan directly as we are now mapped
	;	into the low 48K
	;
	ld d,#0xff		; no subslot
	call testram
	jr not_sub

	;
	;	Register usage here is a bit complex
	;
	;	B = counter
	;	C = bits to make the bank in question appear in the top 16K
	;	    with kernel below
	;	D = subslot number
	;	E = slot numner
	;	H = subslot bits for this subslot with fixed top 16K
	;	L = bits to make the bank in question appear in the low 48K
	;	    with common in the top 16K
	;
	;
	;
	;
sub_scan:
	;
	;	FIXME: We can't map the low 48K to get our subslots the way
	;	we do now because we are running at C000-FFFF. If we map a
	;	different 0x0000-0x3FFF we can't get back because we no
	;	longer have low stubs for our return.
	;
	;	We need to fix this to use high stubs to map the low 16K
	;	scan that, and then use low stubs to map the middle 32K
	;	(high stubs don't work as we have to swap the top 16K to
	;	deal with the brain dead mapping scheme)
	;
	ld l,a			; Remember our A8 bits
	push de			; Save slot number (E)

	; Work out how to get our new slot in the top and the OS below
	ld a,e			; slot
	rrca			; into the top two bits
	rrca
	ld c,a
	in a,(0xA8)
	and #0x3F		; with the old mapping for the low 48K
	or c
	ld c,a			; Mask for selecting our bank for *_sub_slot

	;
	;	Get the subslot for this slot
	;
	ld b,c
	call get_sub_slot
	push af		; Stack it so we can put it back at the end
	;
	ld h,a
	and #0xc0	; Keep the subslot bits for the top 16K
			; as it could be us (in theory anyway - unlikely)
			; low bits are 0 for subslot we want
	ld h,a		; H will count through the masks we need
	ld b,#4		; Number of banks
	ld d,#0		; Count of the banks
next_sub:
	push bc
	ld b,c		; Mask for the bank
	ld a,h		; Subslot mask
	call set_sub_slot
	in a,(0xA8)	; Save the map
	push af
	ld a,l		; Saved A8 mask for mapping this lot low
	out (0xA8),a	; We now have common mapped and low 48K is our subslot
	push hl
	call testram
	pop hl
	pop af
	out (0xA8),a	; Put the map back
	pop bc
	ld a,#0x15		; cycle 00 15 2A 3F (plus fixed top bits)
	add h
	ld h,a
	inc d
	djnz next_sub

	;
	;	Restore the saved subslot.
	;
	pop af
	ld b,c
	call set_sub_slot
	;
	; Recover our slot count
	;
	pop de
	jr sub_done

not_sub:
	ld bc,#18		; move on correct table distance (3 x 6
				; bytes) as no subslots
	add iy,bc
sub_done:
	;
	;	Recover the old slot map state
	;
	pop af
	out (0xA8),a
	;	
	;	Move on a slot
	;
	inc e
	inc ix
	pop bc
	djnz next_slot 

	ret

;
;	See if our 16K bank is writable. If it is then we store it in the
;	table, if not we just bump the pointers
;
;	For 0x4000 and 0x8000 start with a ROM check.
;
writecheck_r:
	ld a,(hl)
	; Check for AB or CD
	cp #'A'
	jr z, romcheck
	cp #'C'
	jr nz, writecheck
romcheck:
	inc a
	inc hl
	cp (hl)
	jr nz, writecheckd
;
;	ROM header: So this 16K chunk isn't RAM
;
	inc hl
	; HL now points at initptr
	inc hl
	inc hl
	inc hl
	inc hl
	; HL now points at device ptr
	ld a,(hl)
	inc hl
	or (hl)
	jr nz, notram
;
;	A device ROM. Potentially interesting. Need to add code to checksum
;	or otherwise identify them later
;
	push de
	push bc
	xor a
	ld l,a			; back to ROM start

	ld d,a			; count
	ld e,a
	ld b,a			; 256 bytes
	ld c,#8			; 2K

nextrombyte:
	ld a,(hl)
	inc hl
	add e
	ld e,a
	jr nc,noinc
	inc d
noinc:
	djnz nextrombyte
	dec c
	jr nz,nextrombyte

	set 7,d			; ROM present flag
	ld (iy),e		; 15 bit sum
	ld 1(iy),d
	pop bc
	pop de
	jr notram

writecheckd:
	dec hl
;
;	For 0x0000 we don't do a ROM check just a r/w check
;
writecheck:
	ld a,#0x55
	ld (hl),a
	cp (hl)
	jr nz, notram
	cpl
	ld (hl),a
	cp (hl)
	jr nz, notram
	; Found one
	; E = slot D = subslot (or FF)
	; BC = table
	ld a,e
	ld (bc),a
	inc bc
	ld a,d
	ld (bc),a
	jr incout
notram:
	inc bc
incout:
	inc bc
	inc iy
	inc iy
	ret
;
;	Scan a slot/subslot for RAM
;
testram:
	ld bc,#_ramtab
	ld hl,#0x1000
	call writecheck
	ld h,#0x40
	call writecheck_r
	ld h,#0x80
	call writecheck_r
	ret

_ramtab:
	.ds 6

;
;	Because the slot changing is so slow and we do bouncing the need
;	for custom copiers is essential. We carefully lay out things so that
;	nothing we need to copy is below the common boundary line.
;

        ; exported symbols
        .globl __uget
        .globl __ugetc
        .globl __ugetw

	.globl outcharhex
	.globl outhl

        .globl __uput
        .globl __uputc
        .globl __uputw
        .globl __uzero

	.globl  map_proc_always
	.globl  map_kernel
;
;	We need these in common as they bank switch
;
        .area _COMMONMEM

;
;	The basic operations are copied from the standard one. Only the
;	blk transfers are different. uputget is a bit different as we are
;	not doing 8bit loop pairs.
;
uputget:
        ; load DE with the byte count
        ld c, 8(ix) ; byte count
        ld b, 9(ix)
	ld a, b
	or c
	ret z		; no work
        ; load HL with the source address
        ld l, 4(ix) ; src address
        ld h, 5(ix)
        ; load DE with destination address (in userspace)
        ld e, 6(ix)
        ld d, 7(ix)
	ret	; 	Z is still false

__uputc:
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_proc_always
	ld (hl), e
uputc_out:
	jp map_kernel			; map the kernel back below common

__uputw:
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	call map_proc_always
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel

__ugetc:
	call map_proc_always
        ld l, (hl)
	ld h, #0
	jp map_kernel

__ugetw:
	call map_proc_always
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel

__uput:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
uput_out:
	call map_kernel
	pop ix
	ld hl, #0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget			; source in HL dest in DE, count in BC
	jr z, uput_out			; but count is at this point magic
	call map_proc_always
	ldir
	jr uput_out

;
__uzero:
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl
	push de
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_proc_always
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
