	;
	; TO9 systems
	;

	.module to9

	; exported
	.globl map_kernel
	.globl map_video
	.globl unmap_video
	.globl map_proc
	.globl map_proc_always
	.globl map_save
	.globl map_restore
	.globl map_for_swap
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
	.globl _need_resched

	.globl _ramsize
	.globl _procmem

	; imported
	.globl unix_syscall_entry
	.globl null_handler

	.globl video_init

	; exported debugging tools
	.globl _plt_monitor
	.globl _plt_reboot
	.globl outchar
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

	include "kernel.def"
	include "../kernel09.def"


	.area .discard
;
;	Get some video up early for debug
;
init_early:
	rts

init_hardware:
	lda	#0x60			; Set the direct page
	tfr	a,dp
	ldd	#128			; for now - need to size properly
	std	_ramsize
	ldd	#128-64			; kernel is 48K but there's a spare
	std	_procmem		; page we can't use right now
	ldd	<$CF			; system font pointer
	subd	#0x00F8			; back 256 as starts at 32 and back
	std	_fontbase		; 8 because it is upside down
	jsr	video_init		; see the video code
	ldd	#unix_syscall_entry	; Hook SWI
	std	<$2F
	rts

        .area .common

_plt_reboot:
	; TODO
_plt_monitor:
	orcc #0x10
	bra _plt_monitor
;
;	Be nice to monitor
;
___hard_irqrestore:		; B holds the data
	tfr b,a
	anda #0x10
	bne hard_di_2
	; fall through
___hard_ei:
	lda $6019
	ora #$20
	sta $6019
	andcc #0xef
	rts
___hard_di:
	tfr cc,b		; return the old irq state
hard_di_2
	lda $6019
	anda #$DF
	sta $6019
	orcc #0x10
	rts

;
; COMMON MEMORY PROCEDURES FOLLOW
;

	.area .common

_program_vectors:
	ldx	#irqhandler
	stx	$6027		; Hook timer
	rts

map_kernel:
	pshs	a
map_kernel_1:
	;	Unlike the TO9+ we don't have to worry about video maps
	;	as the video is only mapped with interrupts off because
	;	of the hairy memory map.
	lda	#0xF0
	sta	kmap
	sta	0xE7C9
	puls	a,pc

map_video:
	pshs	a
	lda	$E7C3
	ora	#1
	sta	$E7C3		;	Video in the low 16K bank
	puls	a,pc

unmap_video:
	pshs	a
	lda	$E7C3
	anda	#0xFE
	sta	$E7C3		;	Video in the low 16K bank
	puls	a,pc

;
;	.commondata is not accessible in the video map case unlike the
;	TO8/TO9+ port.
;
	.area .commondata
kmap:
	.byte	0		; 	A000-DFFF
savemap:
	.byte	0

	.area .common
map_proc_always
	pshs	a
	;	Set the upper page. The low 16K is kernel, the other chunk
	;	is fixed for now until we tackle video.
	lda	U_DATA__U_PAGE+1
	anda	#0xF0
	ora	#0x08
	sta	kmap
	sta	$E7C9		;	Set A000-DFFF, video will be right
	puls	a,pc

map_save:
	pshs	a
	lda	kmap
	sta	savemap
	bra	map_kernel_1

map_restore:
	pshs	a
	lda	savemap
	sta	kmap
	sta	$E7C9
	puls	a,pc

map_for_swap:
	; TODO
	rts


	.area .common
outchar:
	rts

	.area .common

_need_resched:
	.db 0

;
;	Interrupt glue
;
	.area	.common

;
;	Hook the timer interrupt but frob the stack so that we get
;	to run last by pushing a fake short rti frame
;
irqhandler:
	; for a full frame pshs cc,a,b,dp,x,y,u,pc then fix up 10,s 0,s
	; but firstly try Bill  Astle's trick
	ldx	#interrupt_handler
	tfr	cc,a
	anda	#$7F
	pshs	a,x
	jmp	$E830	; will run and then end up in interrupt_handler
;
;	Keyboard glue
;
	.area .text2

	.globl _mon_keyboard
	.globl _mon_mouse
	.globl _mon_lightpen
;
;	This is interlocked by the IRQ paths
;
_mon_keyboard:
	jmp	$E806

;
;	These require the in_bios flag
;
_mon_mouse:
	pshs	y
	jsr	$EC08
	beq	right_up
	lda	#2
	bra	left
right_up:
	lda	#0		; preserve C
left:
	bcc	left_up
	inca
left_up:
	; Now do position
	sta	_mouse_buttons
	jsr	$EC06
	stx	_mouse_x
	sty	_mouse_y
	puls	y,pc

_mon_lightpen:
	pshs	y
	jsr	$E818
	bcs	no_read
	stx	_mouse_x
	sty	_mouse_y
	jsr	$E81B
	lda	#0
	adca	#0
	sta	_mouse_buttons
	ldx	#1
	puls	y,pc
no_read:
	ldx	#0
	puls	y,pc


	.area .common

;
;	Floppy glue
;
;	Set in_bios so we can avoid re-entry between floppy
;	and keyboard scan
;
	.globl _fdbios_flop

_fdbios_flop:
	lda	#1
	sta	_in_bios
	tst	_fd_map
	beq	via_kernel
	; Loading into a current user pages
	jsr	map_proc_always
via_kernel:
	jsr	$E82A
	ldb	#0
	bcc	flop_good
	ldb	<$4E
flop_good
	; ensure map is correct
	tfr	d,x
	clr	_in_bios
	jmp	map_kernel

