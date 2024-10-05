#include "../../cpu-65c816/kernel816.def"

	.export _plt_monitor
	.export _plt_reboot
	.export plt_interrupt_all
	.export init_early
	.export init_hardware
	.export map_kernel
	.export map_kernel_di
	.export map_buffers
	.export map_proc
	.export map_proc_di
	.export map_for_swap
	.export map_proc_a
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export outchar
	.export _program_vectors

	.65c816
	.a16
	.i16

	.common

_plt_monitor:
_plt_reboot:
	stz	$FE78		; map ROM low
	jmp	0

plt_interrupt_all:
init_early:
_program_vectors:
	rts

#define ACIA_RESET	0x03
#define ACIA_RTS_LOW	0x96

	.code

init_hardware:
	lda	#512
	sta	_ramsize
	lda	#448
	sta	_procmem

	; TODO: timer

	; UART
	sep	#0x20
	.a8

	lda	$FEA0
	and	#2
	beq	not_acia
	lda	#ACIA_RESET
	sta	$FEA0
	lda	$FEA0
	and	#2
	bne	not_acia
	inc	_acia_present
	lda	#2
	sta	$FEA0
	lda	#ACIA_RTS_LOW
	sta	$FEA0

not_acia:
	lda	0xFEC3
	and	#0x7F
	tax
	ora	#0x80
	sta	0xFEC3
	lda	0xFEC1
	pha
	lda	#0xAA
	sta	0xFEC1
	cmp	0xFEC1
	bne	no_uart
	txa
	sta	0xFEC3
	lda	0xFEC1
	cmp	#0xAA
	beq	no_uart
	txa
	ora	#0x80
	sta	0xFEC1
	pla
	sta	0xFEC3
	inc	_uart_present
	rep	#0x20
	.a16
	rts

	.a8
no_uart:
	txa
	sta	0xFEC1
	pla
	sta	0xFEC3
	rep	#0x20
	.a16
	rts

	.a16

	.common

map_save_kernel:
	pha
	lda	pagemap
	sta	savemap
	lda	pagemap+2
	sta	savemap+2
	pla
	; Fall through
map_kernel:
map_kernel_di:
map_buffers:
	pha
map_k:
	lda	#0x2021
	sta	pagemap
	sta	0xFE78
	sep	#0x20
	.a8
	lda	#0x22
	sta	pagemap + 2
	sta	0xFE7A
	rep	#0x20
	.a16
	pla
	rts

	; We shouldn't need these
map_proc:
map_proc_di:
	rts

map_for_swap:
map_proc_a:
	pha
	sep	#0x20
	.a8
	sta	pagemap+2	; swap the 8000-BFFF window for swap
	sta	0xFE7A
	rep	#0x20
	.a16
	pla
	rts
	
map_proc_always:
map_proc_always_di:
	pha
	lda	U_DATA__U_PAGE
	sta	pagemap
	sta	0xFE78
	sep	#0x20
	.a8
	lda	U_DATA__U_PAGE + 2
	sta	pagemap+2
	sta	0xFE7A
	rep	#0x20
	.a16
	pla
	rts


map_restore:
	pha
	lda	savemap
	sta	pagemap
	sta	0xFE78
	sep	#0x20
	.a8
	lda	savemap+2
	sta	pagemap+2
	sta	0xFE7A
	rep	#0x20
	.a16
	pla
	rts

pagemap:
	.word	0x2120
	.word	0x2322
savemap:
	.word	0x2120
	.word	0x2322

; outchar is called A8 always
	.a8
outchar:
	pha
	lda	_acia_present
	bne	outu
outa:
	lda	0xFEA0
	and	#2
	beq	outa
	pla
	sta	0xFEA1
	rts
outu:
	lda	0xFEC5
	and	#0x20
	beq	outu
	pla
	sta	0xFEC0
	rts

	.a16

;
;	Hard disk helpers in common space
;
	.export _hd_read_data
	.export _hd_write_data
	.export _hd_map
 
; We don't do swap yet
_hd_read_data:
	lda	_hd_map
	beq	rvia_k
	jsr	map_proc_always
rvia_k:
	ldx	0,y
	iny
	iny

	sep	#0x20
	.a8

	stz	@tmp
rnext:
	lda	0xFE10
	sta	0,x
	inx
	lda	0xFE10
	sta	0,x
	inx
	dec	@tmp
	bne	rnext

	rep	#0x20
	.a16

	jmp	map_kernel
	
_hd_write_data:
	lda	_hd_map
	beq	wvia_k
	jsr	map_proc_always
wvia_k:
	ldx	0,y
	iny
	iny

	sep	#0x20
	.a8

	stz	@tmp
wnext:
	lda	0,x
	sta	0xFE10
	inx
	lda	0,x
	sta	0xFE10
	inx
	dec	@tmp
	bne	wnext

	rep	#0x20
	.a16

	jmp	map_kernel

_hd_map:
	.word	0
