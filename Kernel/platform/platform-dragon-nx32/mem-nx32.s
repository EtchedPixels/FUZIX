;
; memory banking for external memory cartridge
;
; Copyright 2015 Tormod Volden
;

	.module mem_nx32

	; exported
	.globl size_ram
	.globl map_kernel
	.globl map_proc
	.globl map_proc_a
	.globl map_proc_always
	.globl map_save
	.globl map_restore
	.globl copybank

	; imported
	.globl _ramsize
	.globl _procmem
	.globl _membanks
	.globl _internal32k

banksel	equ 0xFFBF
bankoff	equ 0xFFBE
sammap1	equ 0xFFDF
sammap0	equ 0xFFDE

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
	sta sammap1	; internal 64K
	clr ,x
	sta sammap0
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
	sta sammap1	; check for internal 64K
	cmpb ,x		; already same value?
	bne notsame
	incb		; use another value
notsame stb ,x
	cmpb ,x
	bne int32k
	clrb
	inca		; add count for internal upper 32K
int32k	sta sammap0
	stb _internal32k
	sta _membanks
	inca		; add the internal lower 32K to the count
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
	sta sammap0
	puls a,pc

map_proc
	tsta
	beq map_kernel
map_proc_a
	sta map_copy
	bmi map1	; map 128 is for the internal upper 32K
	sta sammap0
	sta banksel
	rts
map1	sta sammap1
	rts

map_proc_always
	pshs a
	lda U_DATA__U_PAGE+1		; LSB of 16-bit page
map_set_a
	sta map_copy
	bmi map1a
	sta sammap0
	sta banksel
	puls a,pc
map1a	sta sammap1
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

; optimized bank copy for fork
; src bank in A, dst bank in B, start in X, end in U
copybank
	pshs dp,a
	lda #0xff
	tfr a,dp
	puls a
	tsta
	bmi copymap1b	; from SAM map 1 to bank B
	tstb
	bmi copyamap1	; from bank A to SAM map 1
	stu endcp
copyf	sta <banksel
	ldu ,x
	stb <banksel
	stu ,x++
	cmpx endcp
	blo copyf
copyret	stb map_copy
	puls dp,pc

copymap1b
	stu endcp
	stb <banksel
copyfb	sta <sammap1
	ldu ,x
	sta <sammap0
	stu ,x++
	cmpx endcp
	blo copyfb
	bra copyret

copyamap1
	stu endcp
	sta <banksel
copyfa	sta <sammap0
	ldu ,x
	sta <sammap1
	stu ,x++
	cmpx endcp
	blo copyfa
	bra copyret

	.area .commondata

endcp		.dw 0
map_store	.db 0
map_copy	.db 0
