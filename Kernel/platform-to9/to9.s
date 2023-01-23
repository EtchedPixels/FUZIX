	;
	; TO9 system routines.
	;
	; Very similar to the TO8 except that the banking is a bit messier
	; and only cartridge space can go in the cartridge area
	;
	; Not yet handled - the extended 64K bank option
	;

	.module to9

	; exported
	.globl map_kernel
	.globl map_video
	.globl map_process
	.globl map_process_always
	.globl map_for_swap
	.globl map_save
	.globl map_restore
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
	lda	#$24			; video from colour plane 40 col mono
	sta	$E7DC
	lda	#0
	sta	$E7DD			; black border
	rts

init_hardware:
	ldd	#192			; for now - need to size properly
	std	_ramsize
	ldd	#192-48			; Kernel has 2,4 and half of 0
	std	_procmem		; 
	ldd	<$CF			; system font pointer
	subd	#0x00F8			; back 256 as starts at 32 and back
	std	_fontbase		; 8 because it is upside down
	jsr	video_init
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
;	sta $6019
	andcc #0xef
	rts
___hard_di:
	tfr cc,b		; return the old irq state
hard_di_2
	lda $6019
	anda #$DF
;	sta $6019
	orcc #0x10
	rts

;
; COMMON MEMORY PROCEDURES FOLLOW
;

	.area .common

_program_vectors:
	; TODO set 2064 to JMP to our irq in setup code
	rts

	; TODO
map_for_swap:
	rts

map_kernel:
	pshs	d,cc
map_kernel_1:
	; This is overkill somewhat but we do neeed to set the video bank
	; for irq cases interrupting video writes.
	; TODO: optimize
	orcc	#$10
	ldb	$E7CB
	andb	#0xFB
	stb	$E7CB
	lda	#0x1F
	sta	kmap
	sta	$E7C9		;	set the A000-DFFF bank
	orb	#$04
	stb	$E7CB
	lda	$E7C3
	anda	#$FE
	tfr	a,b
	anda	#$01
	sta	kmap+2
	stb	$E7C3		;	Ensure kernel half of bank 0 is mapped
	puls	d,cc,pc

map_video:
	pshs	a
	lda	#1
	sta	kmap+2
	lda	$E7C3		;	Map the bitmap bank
	ora	#$01
	sta	$E7C3
	puls	a,pc
	
	.area	.commondata
kmap:
	.byte	0		; 	A000-DFFF
	.byte	0		;	0000-3FFF
	.byte	0		;	Video half bank
savemap:
	.byte	0
	.byte	0
	.byte	0

	.area .common
map_process_always
	pshs	d,cc
	;	Set the upper page. The low 16K is kernel, the other chunk
	;	is fixed.
	lda	U_DATA__U_PAGE
	orcc	#$10
	ldb	$E7CB
	andb	#0xFB
	stb	$E7CB
	sta	kmap
	sta	$E7C9		;	Set A000-DFFF and video bank. Don't
				;	touch the video 8K mapping
	orb	#$04
	stb	$E7CB
	puls	d,cc,pc

map_save:
	pshs	d
	ldd	kmap
	std	savemap
	lda	kmap+2
	sta	savemap+2
	bra	map_kernel_1

map_restore:
	pshs	d,cc
	ldd	savemap
	std	kmap
	ldb	$E7CB
	andb	#0xFB
	stb	$E7CB
	sta	$E7C9
	orb	#$04
	stb	$E7CB
	lda	$E7C3
	anda	#$FE
	ora	kmap+2
	sta	$E7C3
	puls	d,cc,pc

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
	jmp	$E830	; will run and then end up in interrupt_handler

;
;	Keyboard glue
;
	.area .text2

	.globl _mon_keyboard

_mon_keyboard:
	jmp	$E806

	.area .common
;
;	Floppy glue
;
	.globl _fdbios_flop

_fdbios_flop:
	tst	_fd_map
	beq	via_kernel
	; Loading into a current user pages
	jsr	map_process_always
via_kernel:
	jsr	$E82A
	ldb	#0
	bcc	flop_good
	ldb	<$4E
flop_good
	; ensure map is correct
	tfr	d,x
	jmp	map_kernel

