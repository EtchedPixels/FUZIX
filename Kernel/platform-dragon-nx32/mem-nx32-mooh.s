;
; Simple 32K banks on external MOOH memory cartridge
; The 8KB pages are grouped together four at a time
;
; Copyright 2015-2018 Tormod Volden
;

	.module mem_nx32_mooh

	; exported
	.globl size_ram
	.globl map_kernel
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl copybank

	; imported
	.globl _ramsize
	.globl _procmem
	.globl _membanks
	.globl _internal32k

sammap1	equ 0xFFDF
sammap0	equ 0xFFDE
; mmuctl  equ 0xFF90
; mmuen   equ 0x40
; crmen   equ 0x08

	include "kernel.def"
	include "../kernel09.def"

	.area .discard

; Sets ramsize, procmem, membanks
size_ram
	ldx #0x8000	; test location, no code can be preloaded here!
	; We have loaded pages 0-3 with kernel code, so trust these
	; instead of overwriting anything while probing
	lda #4
zpage	sta 0xFFA4  	; clear test byte on the other 8KB pages
	clr ,x
	inca
	cmpa #0x3F	; last page 0x3F cannot be mapped here, so skip it
	blo zpage
	lda #4		; now test writing to pages 4 - 0x3E
	ldb #0xA5	; magic test value
size_next
	sta 0xFFA4
	cmpb ,x		; if value was there already we have probably wrapped around
	beq size_nonram
	stb ,x
	cmpb ,x
	bne size_nonram
	inca
	cmpa #0x3F	; page 0x3F skipped
	bne size_next
size_nonram
	inca		; add page 0x3F that we didn't probe
	lsra		; get result in number of 32KB bytes
	lsra
	ldb #0x3F
* our fork copy code doesn't support the internal bank yet
;	stb 0xFFA4	; expose internal memory
;	sta sammap1	; check for internal 64K
;	ldb ,x
;	cmpb #0x7E	; if no upper 32K we'll see 0x0000 ghosted
;	beq nint32k
;	ldb #0xA5
;	cmpb ,x		; magic value already here?
;	bne notsame
;	incb		; use 0xA6 as magic instead
;notsame stb ,x
;	cmpb ,x
;	bne nint32k
;	clrb
;	inca		; add count for internal upper 32K
* end of disabled code for internal bank
nint32k	sta sammap0
	stb _internal32k	; 0 if there is internal upper 32K
	sta _membanks
	inca		; add the internal lower 32K to the total count
	ldb #32
	mul
	std _ramsize
	subd #64	; what the kernel reserves for itself
	std _procmem
	jsr map_kernel	; leave bank 0 selected and kdata mapped in
	rts

	.area .common

map_kernel
	pshs a
	lda #0
	sta map_copy
	sta $FFA4
	inca
	sta $FFA5
	inca
	sta $FFA6
	inca
	sta $FFA7
	sta sammap0
	puls a,pc

map_process
	tsta
	beq map_kernel
map_process_a
	sta map_copy
	bmi map1	; map 128 is for the internal upper 32K
	pshs a
	sta sammap0
	sta $FFA4
	inca
	sta $FFA5
	inca
	sta $FFA6
	inca
	sta $FFA7
	puls a,pc
map1	sta sammap1
	rts

map_process_always
	pshs a
	lda U_DATA__U_PAGE+1		; LSB of 16-bit page
map_set_a
	sta map_copy
	bmi map1a
	sta sammap0
	sta $FFA4
	inca
	sta $FFA5
	inca
	sta $FFA6
	inca
	sta $FFA7
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

; fast bank copy for fork
; src bank in A, dst bank in B, start in X, end in U
; FIXME support internal bank (SAM map 1)
; FIXME optimize copy loop, should have only one cmp

copybank
	stu endaddr
	stb $FFA4	; map dst at 0x8000
	sta $FFA5	; map src at 0xa000
copyf   cmpx #0x9fff	; address in block? FIXME for non-aligned
	bhi nxtblk
	ldu 0x2000,x	; read from src bank
	stu ,x++	; write to dst bank
	cmpx endaddr
	blo copyf
	bra copydone
nxtblk  inc $FFA4	; next 8KB block
	inc $FFA5
	ldu endaddr
	leau -0x2000,u
	stu endaddr
	leax -0x2000,x
	bra copyf
copydone
	exg b,a		; leave with dst mapped
	jsr map_process_a
	exg b,a
	rts

endaddr		.dw 0
map_store	.db 0
map_copy	.db 0
