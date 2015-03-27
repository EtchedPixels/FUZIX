;
;	Support code for swap device activity
;

		.globl _cartridge_copy
		.globl _cartridge_size
		.globl _sidecar_copy

		.globl _make_mapped

		.globl _map_translate
		.globl _map_live


		.area _COMMONMEM

;
;	A holds the bank to use (1 = RAM 2/3 for cartridge 128+ sidecar
;
_cartridge_copy:
		push bc
		push de
		push hl
		ld de, #0x80	; d is top half of I/O offset, e is the 
				; block counter
		cp #2
		jr z, lowcopy
		ld d, #0x80
lowcopy:
		ld hl, #0x5D00
cart_copy:
		ld a, d
		out (0x11), a	; high byte
		ld bc, #0x10	; port 10, 256 repeats
cart_copy256:
		out (c), b	; set low byte to access
		ld a, (hl)
		out (0x12), a	; write to cartridge
		inc hl
		inc b
		jr nz, cart_copy256
		inc d		; move on a page
		dec e
		jr nz, cart_copy
		pop hl
		pop de
		pop bc
		ret

;
;	Exchange the process in bank a with the process in RAM
;
_cartridge_exchange:
		push bc
		push de
		push hl
		ld de, #0x80	; d is top half of I/O offset, e is the 
				; block counter
		cp #2
		jr z, lowx
		ld d, #0x80
lowx:
		ld hl, #0x5D00
cart_x:
		ld a, d
		out (0x11), a	; high byte
		ld bc, #0x10	; port 10, 256 repeats
cart_x256:
		out (c), b	; set low byte to access
		in a, (0x12)	; existing byte
		push af
		ld a, (hl)
		out (0x12), a	; write to cartridge
		pop af
		ld (hl), a	; write to RAM
		inc hl
		inc b
		jr nz, cart_x256
		inc d		; move on a page
		dec e
		jr nz, cart_x
		pop hl
		pop de
		pop bc
		ret


		.area _CODE

_cartridge_size:
		ld c, #0xA512
		xor a
		out (0x11), a	; low byte 0
		ld a, #0x40
		out (0x10), a	; address 0x4000 selected
		out (c), b
		in a, (0x12)
		cp b
		jr nz, size16
		ld a, #0x80
		out (0x10), a
		out (c), b
		in a, (0x12)
		cp b
		jr nz, size32
		ld hl, #64
		ret
size16:
		ld hl, #16
		ret
size32:
		ld hl, #32
		ret

		.area _COMMONMEM

_sidecar_copy:
		push bc
		push de
		push hl
		and #0x7F	; strip the magic bit for the bank code
		ld e, #0
		rra
		rl e
		ld d, #0x80
		out (0x92), a	; pick the right 64K bank
		ld hl, #0x5D00	; memory area to process
		xor a
		out (0x90), a	; align the low byte on the page boundary
sidecar_cnext:
		ld a, e
		out (0x91), a	; mid part of the address
		ld bc, #0x94	; 256 bytes into 0x94
		otir		; also adjusts HL for us
		inc e		; next page
		dec d		; count of pages to do
		jr nz, sidecar_cnext
		pop hl
		pop de
		pop bc
		ret

_sidecar_exchange:
		push bc
		push de
		push hl
		and #0x7F	; strip the magic bit for the bank code
		ld e, #0
		rra
		rl e
		ld d, #0x80
		out (0x92), a	; pick the right 64K bank
		ld hl, #0x5D00	; memory area to process
sidecar_xnext:
		ld a, e
		out (0x91), a	; mid part of the address
		ld bc, #0x90	; 256 bytes into 0x94
;
;	FIXME: this loop wants optimising properly
;
sidecar_xnl:
		out (c), b	; set low byte
		in a, (0x12)
		out (c), b	; it auto-increments so put it back
		push af
		ld a, (hl)
		out (0x12), a
		pop af
		ld (hl), a
		inc hl
		inc b
		jr nz, sidecar_xnl
		inc e		; next page
		dec d		; count of pages to do
		jr nz, sidecar_xnext
		pop hl
		pop de
		pop bc
		ret

;
;	Make the requested process the current one in memory. We use this
;	on task switching but also on the floppy I/O when swapping in or
;	out. Preserves DE.
;
_make_mapped:
		ld c, a
		ld b, #0
		ld hl, #_map_translate
		add hl, bc
		ld a, (hl)		; Are we in RAM ?
		cp #1
		ret z
		;
		;	We need to do an exchange
		;
		push de
		ld de, (_map_live)	; current RAM owner
		ld (de), a		; current owner gets our page
		ld (hl), #1		; we get its page

		bit 7, a
		jr z, in_sidecar
		;
		;	We are in the cartridge
		;	Swap ourselves with whoever is current RAM task
		;
		call _cartridge_exchange
		jr in_memory2
in_sidecar:
		call _sidecar_exchange
in_memory2:
		pop de			; recover task pointer
		ret
