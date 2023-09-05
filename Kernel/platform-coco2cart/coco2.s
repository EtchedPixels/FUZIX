
	;
	; COCO2 platform
	;

	.module coco2

	; exported
	.globl _mpi_present
	.globl _mpi_set_slot
	.globl _cart_hash
	.globl map_kernel
	.globl map_proc
	.globl map_proc_a
	.globl map_proc_always
	.globl map_save
	.globl map_restore
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
	.globl _need_resched

	.globl _ramsize
	.globl _procmem
	.globl _bufpool
	.globl _discard_size

	; imported
	.globl unix_syscall_entry
;FIX	.globl fd_nmi_handler
	.globl null_handler
	.globl _vtoutput

	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

	; exported debugging tools
	.globl _plt_monitor
	.globl _plt_reboot
	.globl outchar

	include "kernel.def"
	include "../kernel09.def"



	.area .discard

vectors:
	;
	;	At 0x100 as required by the COCO ROM
	;
	jmp badswi_handler			; 0x100
	jmp badswi_handler			; 0x103
	jmp unix_syscall_entry 			; 0x106
	; FIXME: 109 needs to handle bad NMI!
	jmp badswi_handler			; 0x109
	jmp interrupt_handler			; 0x10C
	jmp firq_handler			; 0x10F
vectors_end:

init_early:
	ldx #null_handler
	stx 1
	lda #0x7E
	sta 0
	sta 0x0071		; BASIC cold boot flag (will get trashed by
				; irq stack but hopefully still look bad)
	;
	;	Move the display
	;
	ldx #0xFFC7
	clr ,x+			; Set F0
	clr ,x++		; Clear F1-F6
	clr ,x++
	clr ,x++
	clr ,x++
	clr ,x++
	clr ,x++
	ldy #0x0100
	ldx #vectors
cpvec:
	ldd ,x++
	std ,y++
	cmpx #vectors_end
	bne cpvec
	rts

init_hardware:
	ldd	#64
	std	_ramsize
	ldb	#32
	std	_procmem

	; Turn on PIA  CB1 (50Hz or 60Hz interrupt)
	lda	0xFF03
	ora	#1
	sta	0xFF03
	jsr	_vidtxt
	jsr	_vtinit

	; NTSC or PAL/SECAM ?
	ldx	#0
	lda	$ff02
waitvb0
	lda	$ff03
	bpl	waitvb0		; wait for vsync
	lda	$ff02
waitvb2:
	leax	1,x		; time until vsync starts
	lda	$ff03
	bpl	waitvb2
	stx	_framedet
	rts

	.globl _framedet

_framedet:
	.word	0

        .area .common

_plt_reboot:
	orcc #0x10
	clr 0xFFBE
	lda #0x7e
	sta 0x0071		; in case IRQ left it looking valid
	jmp [0xFFFE]

_plt_monitor:
	orcc #0x10
	bra _plt_monitor

___hard_di:
	tfr cc,b		; return the old irq state
	orcc #0x10
	rts
___hard_ei:
	andcc #0xef
	rts

___hard_irqrestore:		; B holds the data
	tfr b,cc
	rts

;
; COMMON MEMORY PROCEDURES FOLLOW
;

	.area .common
;
;	Our vectors live in a fixed block and are not banked out. Likewise
;	No memory management for now
;
_program_vectors:
	rts

map_kernel:
	pshs a
	lda #1
	sta romin
map_kr:
	clr $ffde	;	ROM in
	puls a,pc
	
map_proc:
map_proc_always:
map_proc_a:
	clr romin
	clr $ffdf	;	ROM out
	rts

map_restore:
	pshs a
	lda romsave
	sta romin
	bne map_kr
	clr $ffdf
	puls a,pc
	
map_save:
	pshs a
	lda romin
	sta romsave
	puls a,pc

	.area .text
;
;	Joystick helper
;
;	jsread(buffer)
;
;	Returns a buffer of words in the format
;	right left/right, button
;	right up/down, button
;	left left/right, button
;	left up/down, button
;
	.globl _jsread

_jsread:
	; Buffer is in X on entry
	pshs u
	lda #$FF
	sta $FF02		; Keyboard scan lines off
	lda #$08		; Select right joystick
	sta $FF23		; Sound off a moment
	bsr jstwo
	lda #$09
	bsr jstwo
	puls u
	rts
jstwo:
	sta $FF03		; P0 CR B - select joystick L or R
	lda #$04
	sta $FF01		; X
	bsr jsfind
	lda #$0C		; Y
	sta $FF01
	; Fall through
jsfind:
	ldu #jstmp
	; Binary search the joystick DAC position
	lda #$20
	sta ,u		; start in the middle and binary search
jssearch:
	lsr ,u
	beq jsdone
	sta $FF20
	tst $FF20
	bpl jsover
	adda ,u
	bra jssearch
jsover:
	suba ,u
	bra jssearch
jsdone:
	ldb $FF20	; save fire button in bit 0
	std ,x++
	rts

	.area .data
jstmp:
	.byte 0

	.area .common
;
;	FIXME:
;
firq_handler:
badswi_handler:
	rti

;
;	debug via printer port
;
outchar:
	sta 0xFF02
	lda 0xFF20
	ora #0x02
	sta 0xFF20
	anda #0xFD
	sta 0xFF20
	rts

	.area .common

_need_resched:
	.db 0
romin:
	.db 0
romsave:
	.db 0

	.area .bufpool
_bufpool:
	.ds 520*5		; initial buffers

	; Discard follows this so will be reclaimed

	.area .discard
_discard_size:
	.db	__sectionlen_.discard__/BUFSIZE
