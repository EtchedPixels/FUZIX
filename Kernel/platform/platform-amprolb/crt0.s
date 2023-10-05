; 2015-02-20 Sergey Kiselev
; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Start with the ROM area CODE-CODE2
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
	; Discard is loaded where process memory wil blow it away
        .area _DISCARD
	; The rest grows upwards from C000 starting with the udata so we can
	; swap in one block, ending with the buffers so they can expand up
        .area _COMMONMEM
	.area _SERIALDATA	; basically common for us
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
	; These get overwritten and don't matter
        .area _INITIALIZER ; binman copies this to the right place for us
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused
	; Page aligned at the top, 256 bytes a port
	.area _SERIAL

        ; exported symbols
        .globl init

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
	.globl s__DATA
        .globl l__DATA
        .globl kstack_top

	.globl interrupt_handler
	.globl nmi_handler

	.include "kernel.def"


	.area _PAGE0 (ABS)

	.org 0


;
;	TODO Put a mini loader here so you can pick between the
;	standard boot and a Fuzix ROM boot
;
;	We blind boot. We should arguably issue TUR commands.
;
bootstrap:
	di
	ld	sp,#0x8200	; in the hole
	call	init_hardware
	call	outs
	.ascii	"Ampro LittleBoard"
	.byte	13,10
	.ascii	"FUZIX Loader"
	.byte	13,10,13,10
	.byte	0
	;	Now load the image. We can't use the main SCSI code as it
	;	has to be in the image as it drives usermem via common space

	in	a,(0x29)
	and	#7
	cp	#7
	jr	nz, no_reset
	call	outs
	.ascii	"Resetting SCSI bus ... "
	.byte	0
	ld	a, #B_RST
	out	(ncr_cmd),a
	ld	b,#0
r_wait:
	djnz	r_wait
	xor	a
	out	(B_RST),a
	ld	hl,#0
re_wait:
	dec	hl
	ld	a,h
	or	l
	jr	nz, re_wait
	; Hopefully all settled
	call	outs
	.ascii	"OK"
	.byte	13,10
	.byte	0
no_reset:
retry:
	xor	a
	out	(ncr_mod),a
	out	(ncr_cmd),a
	out	(ncr_tgt),a

	in	a,(0x29)
	and	#7
	ld	b,a
	ld	a,#1
	jr	z, dev0
self_bit:
	rlca
	djnz	self_bit
;
;	No multimastering so this is the short cut
;
dev0:
	ld	c,a
	or	#1		; target 0
	out	(ncr_data),a
	ld	a,#B_ABUS
	out	(ncr_cmd),a
	ld	a,#B_ASEL|B_ABUS
	out	(ncr_cmd),a

	call	outs
	.ascii	"Drive 0"
	.byte	0
;
;	Wait for selection
;
	ld	hl,#0
selwait:
	in	a,(ncr_bus)
	and	#B_BUSY
	jr	nz, selected
	dec	hl
	ld	a,h
	or	l
	jr	nz, selwait
fail:
	call	outs
	.ascii	" not responding.. retrying."
	.byte	13,10,0
	jp	retry

scsi_read6:
	.byte	0x08
	.byte	0x00
	.byte	0x00
	.byte	0x02		; Start at block 2
	.byte	0x3F		; load the rest of the upper 32K straight
	.byte	0x00

selected:	
	;	Drop SEL keep data on bus
	ld	a,#B_ABUS
	out	(ncr_cmd),a
	xor	a
	;	and then drop bus
	out	(ncr_cmd),a
 	call	outs
	.ascii	": loading image "
	.byte	0
	ld	hl,#0x8200	; Start of load
	ld	de,#scsi_read6	; Read command for lots of blocks
	ld	bc,#ncr_data	; Force a print of the hex, set port
phase:
	in	a,(ncr_bus)
	and	#B_MSG|B_CD|B_IO
	rra
	rra
	out	(ncr_tgt),a
	cp	#1
	jr	z,data_in
	cp	#2
	jr	z,cmdout
	cp	#3
	jr	z, statusin
	call	outs
	.byte	13,10
	.ascii	"Drive 0 in unexpected state - retrying."
	.byte	13,10,0
	jp	retry

statusin:
	ld	hl,#0		; throw status away
data_in:
	in	a,(ncr_bus)
	bit	5,a		; REQ ?
	jr	nz, data_r
	and	#B_BUSY
	jr	z, complete	; Busy dropped = end of command
	jr	data_in		; spin our wheels waiting
data_r:
	in	a,(ncr_st)	; Did the phase change ?
	and	#B_PHAS
	jr	z, phase	; Yes - go to new phase
	ini			; Get data
	ld	a,h
	cp	b		; Have we changed 256 byte block ?
	jr	nz, noout
	ld	b,a		; Remember new page
	call	outh
	ld	a,#8		; Backspace over it
	call	outc
	ld	a,#8
	call	outc
noout:
	ld	a,#B_AACK	; Ack pulse
	out	(ncr_cmd),a
	xor	a
	out	(ncr_cmd),a
	jr	data_in
cmdout:
	ld	a,#B_ABUS
	out	(ncr_cmd),a
	in	a,(ncr_bus)
	bit	5,a		; Wait for REQ
	jr	nz, data_w
	and	#B_BUSY		; Dropped busy means the game is up
	jr	z, complete
	jr	cmdout		; Keep waiting for REQ
data_w:
	in	a,(ncr_st)	; Check we didn't change phase
	and	#B_PHAS
	jp	z,phase
	ld	a,(de)
	out	(c),a		; Put command byte on the bus
	inc	de
	ld	a,#B_AACK|B_ABUS	; Waggle ACK
	out	(ncr_cmd),a
	xor	a
	out	(ncr_cmd),a
	jr	cmdout

complete:
	; TODO - check valid (or do that at 0x500)
	; Put ROM checksum at FFFE/FFFF
	call	outs
	.byte	13,10
	.ascii  "Starting FUZIX"
	.byte	13,10,0
	jp	0x0500

outh:
	push	af
	rra
	rra
	rra
	rra
	call	outhd
	pop	af
outhd:
	and	#0x0F
	add	#'0'
	cp	#'9' + 1
	jr	c, outc
	add	#7
outc:
	push	af
twait:
	xor	a
	out	(DARTA_C),a
	in	a,(DARTA_C)
	and	#0x04
	jr	z, twait
	pop	af
	out	(DARTA_D),a
	ret
outs:
	pop	hl
outsl:
	ld	a,(hl)
	inc	hl
	or	a
	jr	z, outsdone
	call	outc
	jr	outsl
outsdone:	
	jp	(hl)
	
	

;
;	We do things upside down so to speak. Instead of having a ROM boot
;	exist in the ROM after the usual bootstrap code and let that
;	bootstrap boot us or CP/M or whatever. This keeps compatibility
;
;	So when we are called it's from a loader that has put the system
;	into order, loaded the other blocks, mapped the ROM and jumped here.
;
;	In fact it can even be a CP/M app
;
	.area _CODE
init:  
        di
	ld sp,#kstack_top

        ; Zero the data area
        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

;
;	Load sector d from disk into HL and advance HL accordingly
;
load_sector:
	; LOADER TODO
	ret

	.area _COMMONMEM

	.globl	siob_txd
	.globl	siob_status
	.globl	siob_rx_ring
	.globl	siob_special
	.globl	sioa_txd
	.globl	sioa_status
	.globl	sioa_rx_ring
	.globl	sioa_special

	.globl	im2_vectors

;
;	Make sure common memory starts with the im2 vectors
;
im2_vectors:
	.word	0
	.word	0
	.word	0
	.word	interrupt_handler	; CTC 3 vector
	.word	0
	.word	0
	.word	0
	.word	0
	.word	siob_txd
	.word	siob_status
	.word	siob_rx_ring
	.word	siob_special
	.word	sioa_txd
	.word	sioa_status
	.word	sioa_rx_ring
	.word	sioa_special
