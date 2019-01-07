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
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl copybank
	.globl mmu_remap_x

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
	clr map_copy
	clr 0xff91	; MMU task 0
	rts

map_restore
	lda map_store
	beq map_kernel
	; fall-through

* might be called with interrupts enabled from drivers
map_process_always
	pshs cc,a
	lda #1
	orcc #0x10
	sta map_copy
	sta 0xff91	; MMU task 1
	puls cc,a,pc

* called by I/O drivers (also for swapping)
map_process
	cmpx #0
	beq map_kernel
	pshs cc
	orcc #0x10
	jsr map_process_a
	pshs cc,pc

* called directly by switchin
map_process_a
	jsr mmu_remap_x
	lda #1
	sta map_copy
	sta 0xff91	; MMU task 1
	rts

mmu_remap_x
	ldy #0xFFA9
	ldb #7
maplp	lda ,x+
	sta ,y+
	decb
	bne maplp
	rts

map_save
	lda map_copy
	sta map_store
	rts

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
