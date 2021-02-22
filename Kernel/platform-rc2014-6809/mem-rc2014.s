;
; memory banking for RC2104
;

	.module mem_rc2014

	; exported
	.globl size_ram
	.globl map_kernel
	.globl map_process
	.globl map_process_a
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl copybank

	.globl __ugetc
	.globl __ugetw
	.globl __uget
	.globl __uputc
	.globl __uputw
	.globl __uput
	.globl __uzero

	.globl copybanks

	; imported
	.globl _ramsize
	.globl _procmem

	include "kernel.def"
	include "../kernel09.def"

	.area .discard

; Sets ramsize, procmem, membanks
size_ram
	ldd  #512
	std _ramsize
	subd #64	; whatever the kernel occupies (changes when we move
			; to paged)
	std _procmem
	rts

	.area .common

map_kernel
	pshs a
	; FIXME: optimize
	lda #32
	bra map_set_a

map_process
	tsta
	beq map_kernel
map_process_a
	pshs a
	bra map_set_a

map_process_always
	pshs a
	lda U_DATA__U_PAGE+1		; LSB of 16-bit page
map_set_a
	sta curmap
	sta $FE78
	inca
	sta $FE79
	inca
	sta $FE7A
	puls a,pc
	
map_save
	pshs a
	lda curmap
	sta savedmap
	puls a,pc

map_restore
	pshs a
	lda savedmap
	bra map_set_a

;
;	If we could split text/const we could optimzie this because all our
;	consts could be above 0xC000 so available directly from the user map
;	but alas the tool chain can't do it. For now we just do it the slow
;	way but it might well be worth checking in uput/uget so that 99%+ of
;	the time we go the fast path.
;
__ugetc:
	jsr map_process_always
	ldb ,x
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
	jsr map_kernel
	std ,x
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

;	copy bank A into bank B, whole bank. Interrupts are off
;
;	We do 3 x 16K copies keeping the common mapped.
;
copybanks:
	sta $FE79		; source into 0x4000
	stb $FE7A		; dest into 0x8000
	bsr copybank
	inca
	incb
	sta $FE79		; source into 0x4000
	stb $FE7A		; dest into 0x8000
	bsr copybank
	inca
	incb
	sta $FE79		; source into 0x4000
	stb $FE7A		; dest into 0x8000
	bsr copybank
	; Restore the memory map we mashed
	jmp map_kernel
	
;
;	Copy a 16K bank using the MMU paging
; 	This wants a 6309 version
;
copybank:
	pshs d
	ldx #0x4000
	ldu #0x8000
copier:
	ldd ,x++
	std ,u++
	cmpx #0x8000
	bne copier
	puls d,pc


	.area .commondata

curmap		.db 0
savedmap	.db 0
