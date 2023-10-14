;
;	Banking logic for thje RAM192 card (or cards)
;

        .module ram192-bank

        ; exported symbols
        .globl init_hardware
	.globl map_kernel
	.globl map_kernel_restore
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_kernel_di
	.globl map_process_di
	.globl map_process_always_di
	.globl map_process_save
	.globl map_save_kernel
	.globl map_restore
	.globl map_mmio
	.globl unmap_mmio

        ; imported symbols
	.globl _program_vectors	
        .globl _ramsize
        .globl _procmem
	.globl vtbufinit
	.globl ___hard_di
	.globl ___hard_irqrestore
	.globl _sysport
	.globl _card_map

        .include "kernel.def"
        .include "../kernel-z80.def"

	; Has to be in common because we need to bank switch the cards to
	; inspect them
	.area _COMMONMEM

init_hardware:
	call	vtbufinit

	;  Count cards
	ld	a, (_sysport)
	or	#0x80
	out	(0xFE),a

	; Low 48K is now clear of system RAM

	ld	bc,#0x047E		; 4 card slots, at port 7E
	ld	hl,#0x00FF		; some address in the banked RAM
	ld	de,#0x01		; E counts port, D is bitmask of
					; present ports
	xor	a			; Card count
	ex	af,af'			; in A'

next_card:
	out	(c),e			; select card
	ld	a,#0x55
	ld	(hl),a
	cp	(hl)
	jr	nz, not_present
	cpl
	ld	(hl),a
	cp	(hl)
	jr	nz, not_present

	scf
	rl	d
	ex	af,af'
	inc	a
	ex	af,af'
move_on:
	inc	e
	inc	e
	inc	e
	inc	e			; Next card

	djnz	next_card

	; D is now a 4bit mask of present cards
	jr	setup_ram
not_present:
	sla	d
	jr	move_on

setup_ram:
	ld	a,d
	ld	(_card_map),a
	ex	af,af'			; Get count back
	inc	a
	;	We have 192K per card
	ld	de,#192
	ld	hl,#0
	ld	b,a
	jr	ramcountl
ramcount:
	add	hl,de
ramcountl:
	djnz	ramcount
	ld	(_procmem),hl		; 192K per card process memory
	ld	de,#64
	add	hl,de			; And 64K base memory (kernel)
	ld	(_ramsize), hl

        ld	hl, #0
        push	hl
        call	_program_vectors
        pop	hl

        im	1 ; set CPU interrupt mode
        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

map_reg:
	.db	0x00
map_store:
	.db	0x00
port_store:
	.db	0x00

;
;	Map in the kernel.
;
map_kernel_di:
map_kernel:
map_kernel_restore:
	push	af
map_kernel_2:
	; Internal RAM replaces user RAM
	xor	a
	ld	(map_reg),a
	ld	a,(_sysport)
	out	(0xFE),a
	pop	af
	ret

;
;	Select the bank for the relevant process. Update the ksave_map so we
;	can restore the correct kernel mapping when banked.
;
map_process:
map_process_di:
	ld	a, h
	or	l
	jr	z, map_kernel
map_process_hl:
	push	af
	; Set the correct user map first, so that we flip in one neat go
	; to the right user bank with no IRQ holes to worry about
	ld	a,(hl)		; udata page (in common)
map_a:
	ld	(map_reg), a
	out	(0x7E), a
	ld	a,(_sysport)
	or	#0x80		; External memory on
	out	(0xFE),a
	pop	af
	ret

map_process_a:			; used by bankfork
	push	af
	jr	map_a

map_process_save:
map_process_always:
map_process_always_di:
	push	af
	ld	a,(_udata + U_DATA__U_PAGE)
	jr	map_a

map_save_kernel:
	push	af
	ld	a, (map_reg)
	ld	(map_store), a
	ld	a,(_sysport)
	ld	(port_store),a
	jr 	map_kernel_2

; We assume nobody wants to screw with sysport in an IRQ
map_restore:
	push	af
	ld	a,(map_store)
	or	a
	jr	nz, map_a
	ld	a,(port_store)
	ld	(_sysport),a
	ret

map_mmio:
	push	hl
	push	af
	ld	hl,#_sysport
	res	0,(hl)
mmio_out:
	ld	a,(hl)
	out	(0xFE),a
	pop	af
	pop	hl
	ret

unmap_mmio:
	push	hl
	push	af
	ld	hl,#_sysport
	set	0,(hl)
	jr	mmio_out

