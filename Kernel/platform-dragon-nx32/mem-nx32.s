;
; memory banking for external memory cartridge
;
; Copyright 2015 Tormod Volden
;

	.module mem_nx32

	; exported
	.globl size_ram
	.globl map_kernel
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save
	.globl map_restore

	; imported
	.globl _ramsize
	.globl _procmem
	.globl _membanks

banksel	equ 0xFFBF
bankoff	equ 0xFFBE

	include "kernel.def"
	include "../kernel09.def"

	.area .discard

; Sets ramsize, procmem, membanks
size_ram
	ldx #0x8000	; test location, no code can be preloaded here!
	clra
zbank	sta banksel	; clear test byte on all possible banks
	clr ,x
	inca
	cmpa #16
	blo zbank
	clra		; now test writing to banks
	ldb #0xA5
size_next
	sta banksel
	cmpb ,x		; if value was there already we have probably wrapped around
	beq size_nonram
	stb ,x
	cmpb ,x
	bne size_nonram
	inca
	cmpa #16
	bne size_next
size_nonram
	clr banksel	; leave bank 0 selected and kdata mapped in
	sta _membanks
	inca		; add the internal 32K to the count
	ldb #32
	mul
	std _ramsize
	subd #48	; whatever the kernel occupies
	std _procmem
	rts

	.area .common

map_kernel
	pshs a
	lda #0	
	sta map_copy
	sta banksel
	puls a,pc

map_process
	tsta
	beq map_kernel
map_process_a
	sta map_copy
	sta banksel
	rts

map_process_always
	pshs a
	lda U_DATA__U_PAGE+1		; LSB of 16-bit page
map_set_a
	sta map_copy
	sta banksel
	puls a,pc
	
map_save
	pshs a
	lda map_copy
	sta map_store
	puls a,pc

map_restore
	pshs a
	lda map_store
	bra map_set_a

map_store	.db 0
map_copy	.db 0

