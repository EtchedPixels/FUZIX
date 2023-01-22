
	;
	;	MO6 - much like a TO8 with shifted address maps
	;

	.module mo6

	; exported
	.globl map_kernel
	.globl map_video
	.globl map_process
	.globl map_process_always
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
	pshs	d
map_kernel_1:
	; This is overkill somewhat but we do neeed to set the video bank
	; for irq cases interrupting video writes.
	;
	; TODO: we'll need to flip between cartridge and video once video
	; is user mapped high
	;
	ldd	#0x0200		;	FIXME - what is the right value for memo7 map
	std	kmap
	std	$A7E5		;	set the 6000-9FFF to 2
	anda	#$01
	sta	kmap+2
	stb	$A7C3		;	Ensure kernel half of bank 0 is mapped
	puls	d,pc

map_video:
	pshs	a
	lda	#1
	sta	kmap+2
	lda	$A7C3		;	Map the bitmap bank
	ora	#$01
	sta	$A7C3
	puls	a,pc

	.area	.commondata
kmap:
	.byte	0		; 	6000-9FFF
	.byte	0		;	B000-EFFF
	.byte	0		;	Video half bank
savemap:
	.byte	0
	.byte	0
	.byte	0

	.area	.common

map_process_always
	pshs	a
	;	Set the upper page. The low 16K is kernel, the other chunk
	;	is fixed.
	ldd	U_DATA__U_PAGE
	std	kmap
	std	$A7E5		;	Set A000-DFFF and video bank. Don't
				;	touch the video 8K mapping
	puls	a,pc

map_save:
	pshs	d
	ldd	kmap
	std	savemap
	lda	kmap+2
	sta	savemap+2
	bra	map_kernel_1

map_restore:
	pshs	d
	ldd	savemap
	std	kmap
	std	$A7E5
	lda	$A7C3
	anda	#$FE
	ora	kmap+2
	sta	$A7C3
	puls	d,pc


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
	swi
	.byte	0x0A
	beq	nokey
	tfr	d,x
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
	jsr	map_process_always
via_kernel:
	swi
	.byte	0x26
	ldb	#0
	bcc	flop_good
	ldb	<$4E
flop_good
	; ensure map is correct
	tfr	d,x
	jmp	map_kernel
;
;	SD glue
;
	.globl _sd_spi_raise_cs
	.globl _sd_spi_lower_cs
	.globl _sd_spi_transmit_byte
	.globl _sd_spi_receive_byte
	.globl _sd_spi_transmit_sector
	.globl _sd_spi_receive_sector

_sd_spi_raise_cs:
	rts
_sd_spi_lower_cs:
	rts
_sd_spi_transmit_byte:
	rts
_sd_spi_receive_byte:
	rts
_sd_spi_transmit_sector:
	rts
_sd_spi_receive_sector:
	rts
