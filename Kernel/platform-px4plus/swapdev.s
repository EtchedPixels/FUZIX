;
;	Support code for mmio RAM based swap devices used as if they were
;	banked memory. We could have made them swap but its faster to treat
;	them as memory and do switching and exchanges. It also allows us to
;	avoid the cost of having an empty space as needed for swap out and
;	then swap in. On a PX4 with a single 32K cartridge that is critical.
;
;	The key element here is the map translation table. Each of the
;	logical banks has a 1 byte entry that gives its current location
;	as either
;		1 - RAM
;		2 - Cartridge, low 32K
;		3 - Cartridge, high 32K (if present)
;		0x80-0x83 - Sidecar in 32K chunks
;
;	When we "switch" to a bank we actually swap the RAM contents with
;	the I/O memory that held the bank we switched to, and then we fix
;	up the table. That makes all the magic going on conveniently
;	invisible to the core code.
;

		.globl _cartridge_copy
		.globl _cartridge_size
		.globl _sidecar_copy

		.globl _make_mapped

		.globl _map_translate
		.globl _map_live

		.globl _read_from_bank

		.globl _romd_addr
		.globl _romd_off
		.globl _romd_size
		.globl _romd_mode	; byte size

		.globl map_process_save
		.globl map_kernel_restore

		.module swapdev

		.include "kernel.def"

		.area _COMMONMEM

;
;	Copy the RAM into the given bank in the cartridge. This is used when
;	we do a fork.
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
;	Exchange the process in bank a with the process in RAM. This is
;	used when we do a task switch between two processes.
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
;
;	Compute the size of a cartridge. We can only use 32K sized chunks,
;	so the 16K cartridge while detected is useless.
;
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

;
;	A holds the sidecar bank to use with bit 7 set. We call this
;	function when we are forking a process so that we can write a copy
;	of RAM into the other bank (where it will become the parent)
;
_sidecar_copy:
		push bc
		push de
		push hl
		ld d, a		; save bank
		in a, (0x94)
		or #0x02	; memory power save off
		out (0x94), a
		ld a, #0x7F	; strip the magic bit for the bank code
		and d
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
		in a, (0x94)
		and #0xFD	; memory power save on
		out (0x94), a
		pop hl
		pop de
		pop bc
		ret

;
;	Exchange RAM and the process in sidecar bank A (with bit 7 set).
;	Used when we are task switching. This is rather ugly as the
;	sidecar memory helpfully auto-increments the low byte!
;
_sidecar_exchange:
		push bc
		push de
		push hl
		ld d, a		; save bank
		in a, (0x94)
		or #0x02	; power save off
		out (0x94), a
		ld a, #0x7F	; strip the magic bit for the bank code
		and d
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
		in a, (0x94)
		and #0xFD	; memory power save on
		out (0x94), a
		pop hl
		pop de
		pop bc
		ret

;
;	Make the requested bank the current one in memory. We use this
;	when task switching. It's the PX4 logical equivalent of the out
;	instructions on saner ports that switch context. It must be called
;	on a private stack as it will exchange the kernel/irq stacks of the
;	old and new task as it runs.
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

;
;	We have to deal with the awkward case of I/O wanting to access other
;	banks for swapping. On a normal setup this is fairly trivial, in our
;	case the memory they want to copy may in fact be stuffed into an I/O
;	mapped device so we need to do some gymnastics.
;
_read_from_bank:	; (page, dptr, buf) always 128 bytes
		ld de, (_romd_mode)
		ld d, #0
		ld hl, #_map_translate
		add hl, de
		ld a, (hl)		; Translation entry
		cp #1
		jr z, memory_read	; It is in RAM
		;
		; Turn virtual address into offset in swap in HL as both
		; other methods need this
		;
		ld hl, (_romd_off)	; in bytes, userspace ptr
		ld de, #SWAP_VTOP
		or a
		sbc hl, de		; convert to zero based
		bit 7, a		; cartridge is 2/3
		jr z, cartridge_read
		;
		; Pull it from the sidecar (0x80-0x83)
		;
		ld d, a			; save bank
		in a, (0x94)
		or #0x02		; power save off
		out (0x94), a
		ld a, #0x7F		; strip the magic bit for the bank code
		and d
		ld b, #0
		rra			; effectively multiply by 32768
		rr b			; 0x0 or 0x80
		and #0x3f		; 32K per process
		out (0x92), a		; select right 64K bank
		;
		; Now work out which 256 byte page is needed
		;
		ld a, h
		add b			; 32K low bit of bank number
		out (0x91), a		; middle byte of I/O offset
		ld a, l
		out (0x90), a		; low byte of I/O offset
		ld bc, #0x8093		; 128 count, port 92)
		ld hl, (_romd_addr)
		inir			; 128 bytes into hl
		in a, (0x94)
		and #0xFD		; memory power save on
		out (0x94), a
		ret
;
;		2 is the low 32K, 3 the high
;
cartridge_read:
		cp #2
		jr z, cart_read_1
		set 7,h			; offset by another 32K
cart_read_1:
		ld a, h
		out (0x10), a		; set high byte
		ld de, (_romd_addr)
		ld b, #0x80		; 128 bytes
cart_read_l:
		ld a, l			; sufficient as this won't
		out (0x11), a		; cross a page
		in a, (0x12)
		ld (de), a
		inc de
		inc l
		djnz cart_read_l
		ret
;
;		Pull 128 bytes from user space. We pull a trick here.
;		We are copying between user space and kernel writable data.
;		Only R/O space is banked on the PX4 systems so we don't need
;		to play bank pogo.
;
memory_read:
		ld bc, #0x80
		ld de, (_romd_addr)
		call map_process_save
		ldir
		call map_kernel_restore
		ret

_map_translate:	.ds MAX_MAPS
_map_live:	.dw 0
