
#include "../kernel-68000.def"

		.globl sd_spi_transmit_byte
		.globl sd_spi_receive_byte
		.globl sd_spi_transmit_sector
		.globl sd_spi_receive_sector
		.globl sd_spi_clock
		.globl sd_spi_raise_cs
		.globl sd_spi_lower_cs

.mri 1

;
;	Raise the CS line for the SD card
;
sd_spi_init:
sd_spi_raise_cs:
		move.b	#$0C,$100AE	; select device 3 (raise cs on all)
sd_spi_clock:
		rts

;
;	Select card n (0-2)
;
sd_spi_lower_cs:
		move.b	sd_drive,d0
		move.b	#$0C,$1000E	; device 3
		eor.b	#3,d0
		rol.b	#2,d0
		move.b	d0,$1000F	; clear bits to select our device
		rts

;
;	Transfer routines. We don't implement a bidirectional exchange
;	as none of our devices currently need it.
;

;
;	Put everything into registers for speed and simplicity
;
spi_setup:
		lea	$100AF,a1		; clear bit address
		lea	$100AD,a2		; input address
		lea	$100AE,a3		; set bit address
		move.b	#$40,d1		; clock bit
		move.b	#$80,d2		; data bit
		move.b	#$C0,d3		; both bits
		rts

;
;	The SC26C92 has registers for set bits and clear bits so this
;	looks a bit different to things like the Z80 PIO
;
spi_bitbang_tx:
		moveq	#7,d4
spi_bb_tx:
		add.b	d0,d0
		bcc	spi_tx0
		; lower clock bit
		move.b	d1,(a3)		; low | 1
		; raise data bit
		move.b	d3,(a1)
		; raise clock bit
		move.b	d1,(a1)		; high | 1 (sample)
		dbra.w	d4,spi_bb_tx 
		rts
spi_tx0:
		; lower clock and data
		move.b	d2,(a3)		; low | 0
		; raise clock bit
		move.b	d1,(a1)		; high | 0 (sample)
		dbra.w	d4,spi_bb_tx
		rts

;
;	This needs work. The 68000 BTST changes the Z flag which is not
;	what we really want for speed.  Bit 7 is always set so adding 64
;	will give us the needed carry but is still a mess.
;
spi_bitbang_rx:
		move.b	d3,(a1)		; data to high (and kept there)
		; Bit 7
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 6
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 5
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 4
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 3
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 2
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 1
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		; Bit 0
		move.b	d1,(a3)		; clock low
		move.b	d1,(a1)		; clock high
		move.b	(a2),d4		; test input
		add.b   d5,d4
		add.b	d0,d0
		rts

;
;	Transmit a 512 byte SD card sector to the media. This is the bit
;	that needs to be optimised for speed.
;
sd_spi_transmit_sector:
		movem.l a2-a3/d2-d5,-(sp)
		move.l blk_op,a0	; blk_op.addr
		bsr spi_setup
		move.w #511,d5
spi_txb:
		move.b	(a0)+,d0
		bsr spi_bitbang_tx
		dbra.w d5,spi_txb
		movem.l (sp)+,a2-a3/d2-d5
		rts

;
;	Receive a 512 byte burst from the SD card, even more so speed is
;	important here.
;
sd_spi_receive_sector:
		movem.l a2-a3/d2-d6,-(sp)
		move.l blk_op,a0	; blk_op.addr
		bsr spi_setup
		move.b #64,d5		; used for rx bit to carry
		move.w #511,d6
spi_rxb:
		move.b	(a0)+,d0
		bsr spi_bitbang_rx
		dbra d6,spi_rxb
		movem.l (sp)+,a2-a3/d2-d6
		rts

;
;	C wrapper for transmit and receive of a byte - not particularly
;	speed sensitive.
;	
sd_spi_transmit_byte:
		move.b 7(sp),d0
		movem.l a2-a3/d2-d5,-(sp)
		bsr spi_setup
		bsr spi_bitbang_tx
		movem.l (sp)+,a2-a3/d2-d5
		rts

sd_spi_receive_byte:
		movem.l a2-a3/d2-d6,-(sp)
		bsr spi_setup
		move.b #64,d5
		bsr spi_bitbang_rx
		movem.l (sp)+,a2-a3/d2-d6
		rts
