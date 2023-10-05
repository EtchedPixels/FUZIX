
	;
	;	MO6 - much like a TO8 with shifted address maps
	;

	.module mo6

	; exported
	.globl map_kernel
	.globl map_video
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
	ldd	#128			; for now - need to size properly
	std	_ramsize
	ldd	#64			; Kernel has 2,4 and half of 0
	std	_procmem		; will be 2,3,4 eventually
	ldd	<$CF			; system font pointer
	subd	#0x00F8			; back 256 as starts at 32 and back
	std	_fontbase		; 8 because it is upside down
	jsr	video_init
	ldd	$205E
	std	monswi
	ldd	#swivec-0x10		; rework the SWI vectors for syscall
	std	$205E			; steal SWI
	ldx	#$2061
	lda	2,x
	bne	oldint
	ldd	#interrupt_handler
	std	,x
	lda	#1
	sta	2,x
	bra	intdone
oldint:
	; Chain system int but the tricky way as we must run last
	ldd	,x
	std	oldintvec + 1
	ldd	#irqhandler
intdone:
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
	lda $2019
	ora #$20
	sta $2019
	andcc #0xef
	rts
___hard_di:
	tfr cc,b		; return the old irq state
hard_di_2
	lda $2019
	anda #$DF
	sta $2019
	orcc #0x10
	rts

;
; COMMON MEMORY PROCEDURES FOLLOW
;

	.area .common

_program_vectors:
	; TODO set ?? 2064 to JMP to our irq in setup code
	rts
;
map_kernel:
	pshs	a
map_kernel_1:
	;
	; TODO: we'll need to flip between cartridge and video once video
	; is user mapped high
	;
	lda	#0x02
	sta	kmap
	sta	$A7E5		;	set 6000-9FFF to 2 (kernel)
	puls	a,pc

map_video:
	pshs	a
	lda	#3
	sta	kmap
	sta	$A7E5		;	set 6000-9FFF to 3 (video0
	puls	a,pc

	.area	.commondata
kmap:
	.byte	0		; 	6000-9FFF
savemap:
	.byte	0

	.area	.common

map_proc_always
	pshs	a
	;	Set the upper page. The low 16K is kernel, the other chunk
	;	is fixed.
	ldd	U_DATA__U_PAGE
	sta	kmap
	sta	$A7E5		;	Set 6000-9FFF and video bank. Don't
				;	touch the video 8K mapping
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
	sta	$A7E5
	puls	d,pc

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
;	6021 is the vector for irqpt 6027 for timept - seems it takes one or
;	the other according to what is going on. May need to dump and trace
;	monitor to see how to clean up the stack our way
;
irqhandler:
	; for a full frame pshs cc,a,b,dp,x,y,u,pc then fix up 10,s 0,s
	; but firstly try Bill  Astle's trick
	ldx	#interrupt_handler
	tfr	cc,a
	anda	#$7F
	pshs	a,x
oldintvec:
	jmp	$1234	; will run and then end up in interrupt_handler

;
;	Keyboard glue
;
	.area .text2

	.globl _mon_keyboard

_mon_keyboard:
	ldx	monswi
	stx	$205E
	swi
	.byte	0x0A
	beq	nokey
	tfr	d,x
	ldx	#swivec
	stx	$205E
	rts
nokey:
	ldx	#0
	rts

	.area .common
;
;	Floppy glue
;
	.globl _fdbios_flop

_fdbios_flop:
	tst	_fd_map
	beq	via_kernel
	; Loading into a current user pages
	jsr	map_proc_always
via_kernel:
	ldx	monswi
	stx	$205E
	swi
	.byte	0x26
	ldb	#0
	bcc	flop_good
	ldb	<$4E
flop_good
	ldx	#swivec
	stx	$205E
	; ensure map is correct
	tfr	d,x
	jmp	map_kernel

	.area .commondata

swivec:	.word	unix_syscall_entry
monswi:	.word	0
