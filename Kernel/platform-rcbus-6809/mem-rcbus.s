;
; memory banking for RC2104
;

	.module mem_rcbus

	; exported
	.globl size_ram
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl map_for_swap

	.globl __sectionbase_.common__

	.globl cur_map

	.globl __ugetc
	.globl __ugetw
	.globl __uget
	.globl __uputc
	.globl __uputw
	.globl __uput
	.globl __uzero

	.globl _copy_common

	; imported
	.globl _ramsize
	.globl _procmem

	include "kernel.def"
	include "../kernel09.def"

	.area .discard

; Sets ramsize, procmem, membanks
size_ram:
	ldd  #512
	std _ramsize
	subd #48	; whatever the kernel occupies (changes when we move
	std _procmem
	rts

	.area .common

map_process:
	cmpx	#0
	bne	map_procu
map_kernel:
	pshs	d
	ldd	#0x2021
	std	cur_map
	std	0xFE78
	incb
	stb	cur_map+2
	stb	0xFE7A
	puls	d,pc

map_procu:
	pshs	d
	ldd	,x++
	std	cur_map
	std	$FE78
	ldb	,x
	stb	cur_map+2
	stb	$FE7A
	puls	d,pc

map_process_always:
	pshs	d
	ldd	U_DATA__U_PAGE
	std	cur_map
	std	0xFE78
	ldb	U_DATA__U_PAGE+2
	stb	cur_map+2
	stb	0xFE7A
	puls	d,pc
	
map_save:
	pshs	d
	ldd	cur_map
	std	saved_map
	ldb	cur_map+2
	stb	saved_map+2
	puls	d,pc

map_restore:
	pshs	d
	ldd	saved_map
	std	cur_map
	std	0xFE78
	ldb	saved_map+2
	stb	cur_map+2
	stb	0xFE7A
	puls	d,pc

map_for_swap:
	sta	cur_map+1
	sta	0xFE79
	rts

_copy_common:
	;	B holds the page to stuff it in, ints are off, and we will
	;	clean up all the maps in the caller later. Will need
	;	review if we allow interrupts during fork copy
	stb	$FE79
	ldx	#__sectionbase_.common__	; Start of common space to copy
	ldy	#__sectionbase_.common__-0x8000	; we are mapping a Cxxx page at 4xxx
commoncp:
	ldd	,x++
	std	,y++
	cmpx	#$FE00		; I/O window
	beq	skipio
	cmpx	#0		; copy until we did all the vectors
	bne	commoncp
	jmp	map_kernel
skipio:
	leax	0x100,x
	leay	0x100,y
	bra	commoncp

__ugetc:
	jsr map_process_always
	ldb ,x
	clra
	tfr d,x
	jmp map_kernel
__ugetw:
	jsr map_process_always
	ldx ,x
	jmp map_kernel
__uget:
	pshs u,y
	ldu 6,s
	ldy 8,s
ugetl:
	jsr map_process_always
	lda ,x+
	jsr map_kernel
	sta ,u+
	leay -1,y
	bne ugetl
	ldx #0
	puls u,y,pc

__uputc:
	ldd 2,s
	jsr map_process_always
	exg d,x
	stb ,x
	jsr map_kernel
	ldx #0
	rts
__uputw:
	ldd 2,s
	jsr map_process_always
	exg d,x
	std ,x
	jsr map_kernel
	ldx #0
	rts
;
;	Might be worth doing word sized loops for uput/uget ?
;
__uput:
	pshs u,y
	ldu 6,s
	ldy 8,s
uputl:
	jsr map_kernel
	lda ,x+
	jsr map_process_always
	sta ,u+
	leay -1,y
	bne uputl
	jsr map_kernel
	ldx #0
	puls u,y,pc

__uzero:
	pshs y
	ldy 4,s
	jsr map_process_always
	tfr y,d
	clra
	lsrb
	bcc evenc
	sta ,x+
	leay -1,y
	beq zdone
evenc:
	clrb
uzloop:
	std ,x++
	leay -2,y
	bne uzloop
zdone:
	jsr map_kernel
	ldx #0
	puls y,pc

	.area .commondata

cur_map:
	.dw 0
	.db 0
saved_map:
	.dw 0
	.db 0
