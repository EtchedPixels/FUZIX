;
; Flexible 8K banks on external MOOH memory cartridge
;
; Copyright 2015-2018 Tormod Volden
;

	.module mem_mooh

	; exported
	.globl reloc
	.globl size_ram
	.globl map_kernel
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
	.globl start

; mmuctl  equ 0xFF90
; mmuen   equ 0x40
; crmen   equ 0x08
; mmutask equ 0xFF91

	include "kernel.def"
	include "../kernel09.def"

	.area .discard

; Sets ramsize, procmem, membanks
size_ram
	; we know what we should have
	lda #56
	sta _membanks	; not really used anywhere
	ldd #512
	std _ramsize	; in KB
	ldd #512-64
	std _procmem	; minus 64KB for kernel
	rts

	.area .common

; must preserve register a but not x
map_kernel
	pshs cc
	clr map_copy
	clr map_copy+1
	clr 0xff91	; MMU task 0
	puls cc,pc

map_process_a
	pshs cc,a,b,x,y
	bra map_x

* might be called with interrupts enabled from drivers
map_process_always
	pshs cc,a,b,x,y
	orcc #0x10
	ldx U_DATA__U_PAGE
map_x
	; could try to skip this if maps already in place
	stx map_copy
	ldy #0xFFA9
	ldb #7
maplp	lda ,x+
	bmi fillm	; PAGE_INVALID (0xFF) means we are done
	sta ,y+
	decb
	bne maplp
mapdn	lda #1
	sta 0xff91	; MMU task 1
	puls cc,a,b,x,y,pc
fillm	lda -2,x	; should not happen on first page
fillp	sta ,y+		; fill map by repeating last page
	decb
	bne fillp
	bra mapdn

map_save
	pshs x
	ldx map_copy
	stx map_store
	puls x,pc

map_restore
	pshs x
	ldx map_store
	bne usr
	jsr map_kernel
	puls x,pc
usr	jsr map_process_a
	puls x,pc


; fast bank copy for fork (called with kernel mapped)
; src bank in A, dst bank in B

; we simply copy the full 8KB bank here

copybank
	pshs x,y,u
	ldy 0xFFA4	; save the two MMU regs
	sta 0xFFA5	; map src at 0xA000 (task 0, kernel mapped)
	stb 0xFFA4	; map dst at 0x8000
	ldu #0xA000
	ldx #0x8000

copylp	ldd ,u++	; read from src bank
	std ,x++	; write to dst bank
	ldd ,u++
	std ,x++
	ldd ,u++
	std ,x++
	ldd ,u++
	std ,x++
	ldd ,u++
	std ,x++
	ldd ,u++
	std ,x++
	ldd ,u++
	std ,x++
	ldd ,u++
	std ,x++
	cmpx #0xA000
	blo copylp

	sty 0xFFA4	; restore MMU regs
	puls x,y,u,pc

	.area .commondata

map_store	.dw 0
map_copy	.dw 0
