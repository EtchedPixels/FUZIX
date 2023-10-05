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
	.globl map_proc
	.globl map_proc_a
	.globl map_proc_always
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
map_proc_always
	pshs cc,a
	lda #1
	orcc #0x10
	sta map_copy
	sta 0xff91	; MMU task 1
	puls cc,a,pc

* called by I/O drivers (also for swapping)
map_proc
	cmpx #0
	beq map_kernel
	pshs cc
	orcc #0x10
	jsr map_proc_a
	pshs cc,pc

* called directly by switchin
map_proc_a
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
	pshs	dp,x,y,u
	ldx	0xFFA4		; save the two MMU regs on stack
	pshs	x
	std	0xFFA4		; map src at 0x8000 (MMU task 0, kernel mapped), map dst at 0xA000
	sts	save_sp
	lds	#0xC000		; top of DEST
	ldu	#0xA000+7	; top of SRC
smash@	leau	-14,u
	pulu	dp,d,x,y	; transfer 7 bytes at a time
	pshs	dp,d,x,y	; 6 times.. 42 bytes per loop
	leau	-14,u
	pulu	dp,d,x,y
	pshs	dp,d,x,y
	leau	-14,u
	pulu	dp,d,x,y
	pshs	dp,d,x,y
	leau	-14,u
	pulu	dp,d,x,y
	pshs	dp,d,x,y
	leau	-14,u
	pulu	dp,d,x,y
	pshs	dp,d,x,y
	leau	-14,u
	pulu	dp,d,x,y
	pshs	dp,d,x,y
	cmpu	#0x8002+42+7	; end of copy? (leave space for interrupt) - bottom of SRC
	bne	smash@		; no repeat
	ldx	save_sp		; put stack back
	exg	x,s		; and data DEST ptr to X now
	leau	-7,u
safe@	ldd	,--u		; move last 44 bytes with a normal stack
	std	,--x		; 4 bytes per loop
	ldd	,--u
	std	,--x
	cmpx	#0xA000		; reached bottom of DEST?
	bne	safe@
	puls	x
	stx	0xFFA4		; restore MMU regs
	puls	dp,x,y,u,pc

	.area .commondata

map_store	.dw 0
map_copy	.dw 0
save_sp		.dw 0
