
#include "../../cpu-68000/kernel-68000.def"

		.globl plt_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl interrupt
		.globl plt_idle
		.globl outchar
		.globl plt_monitor

		.globl sd_spi_receive_byte
		.globl sd_spi_transmit_byte
		.globl sd_spi_receive_sector
		.globl sd_spi_transmit_sector

.mri 1
plt_monitor:
plt_reboot:
	    or #$0700,sr
	    move.l 0,sp
	    move.l 4,a0
	    reset
	    jmp (a0)

init_early:
	    lea.l udata_block,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

plt_idle:
	    stop #($2000+EI_MASK)
	    rts
init_hardware:
	    move.w #128,d0
	    move.w d0,ramsize
	    move.l #_end+512,d1
	    sub.l #$10000,d1		; Base of RAM
	    lsr.l #8,d1
	    lsr.l #2,d1
	    sub.w  d1,d0		; Guess for kernel
	    move.w d0,procmem		; guesses for now

            rts

interrupt:
	    ; C will save and restore a2+/d2+
	    movem.l a0-a1/a5/d0-d1,-(sp)
	    move.l udata_shadow,a5	; set up the register global
	    move.b #1,U_DATA__U_ININTERRUPT(a5)
	    jsr plt_interrupt
	    clr.b U_DATA__U_ININTERRUPT(a5)

	    tst.b U_DATA__U_INSYS(a5)
	    bne no_preempt
	    tst.b need_resched
	    beq no_preempt
	    ;
	    ;	Vanish into the scheduler. Some other task will pop back out
	    ;   and eventually we'll re-appear here and continue.
	    ;
	    ;   FIXME: check IRQ masking
	    ;
	    move.l U_DATA__U_PTAB(a5),a0
	    ; We might be in STOPPED state in which case do not go back
	    ; to ready !
	    cmp.b #P_RUNNING,P_TAB__P_STATUS_OFFSET(a0)
	    bne no_ready
	    move.b #P_READY,P_TAB__P_STATUS_OFFSET(a0)
	    ori.b #PFL_BATCH,P_TAB__P_FLAGS_OFFSET(a0)
no_ready:
	    bsr switchout
no_preempt:
	    tst.b U_DATA__U_CURSIG(a5)
	    beq no_signal
	    ;   FIXME: this is ugly
	    movem.l (sp)+,a0-a1/a5/d0-d1
	    move.w sr,-(sp)
	    move.w #0,-(sp)
	    movem.l a0-a6/d0-d7,-(sp)
	    move.l (sp),-(sp)
	    bsr exception
	    addq #4,sp
	    movem.l (sp)+,a0-a6/d0-d7
	    addq #4,sp
	    rte
no_signal:
	    movem.l (sp)+,a0-a1/a5/d0-d1
	    rte

;
;	Nothing to do in 68000 - all set up once at link time
;
program_vectors:
	rts
;
;	We do no banking so we need to do nothing here.
;
map_process_always:
map_process:
map_kernel:
map_restore:
map_save:
	rts

; outchar: Wait for UART TX idle, then print the char in d0

outchar:
outcharw:
	; FIXME
	rts

;
;	Bitbangers for the SPI bus
;
;	There may be clever things we can here using the shift reg
;	or the CA/CB pulses for speed. Needs thought and some
;	experimentation. Ditto clocking off T1 and counting
;	cycles
;
;
;	Receive a byte not caring about what we send (FF)
;
spi_rx_byte:
	movea.l #$30002,a0		; VIA data A
	move.b	(a0),d1			; Preserve other output bits
	move.b	d1,d2
	or.b	#$02,d1			; Clock high in D1
	and.b	#$FD,d2			; Clock low in D2
	moveq	#1,d4
	; Entry point if already have the precalc values
spi_rx_next:
	; Unrolled for speed for SD card use
	; Send 1 bits
	; Bit 0
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	; Bus should now hold our bit to sample
	move.b	(a0),d3			; Bit 7
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 1
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 2
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 3
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 4
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 5
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 6
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	; Bit 7
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a0),d3
	lsl.b	d4,d3
	roxl.b	d4,d0
	rts

spi_tx_byte:
	move.l	#$30002,a0		; VIA data A
	moveq	#1,d3
	moveq	#$02,d4
spi_tx_next:
	move.b	(a0),d1
	or.b	#3,d1			; Set clock, data
	moveq	#7,d2
spi_tx_bit:
	lsr.b	d3,d1			; data right
	roxl.b	d3,d0			; Data bit
	roxl.b	d3,d1			; Into the low bit
	sub.b	d4,d1			; Minus clock
	move.b	d1,(a0)
	add.b	d4,d1			; Plus clock
	move.b	d1,(a0)
	dbra.w	d2,spi_tx_bit
	rts

;
;	Wrappers for C calling
;
sd_spi_receive_byte:
	movem.l	d2-d4,-(sp)
	bsr	spi_rx_byte
	movem.l	(sp)+,d2-d4
	rts

sd_spi_transmit_byte
	movem.l	d2-d4,-(sp)
	move.l	16(sp),d0
	bsr	spi_tx_byte
	movem.l	(sp)+,d2-d4
	rts

;
;	For an SD card this is the most critical path
;
sd_spi_receive_sector:
	movem.l	a2/d2-d5,-(sp)
	; Get the argument - address of buffer
	; As we are a flat system we don't have to worry about
	; kernel v user etc
	move.l	24(sp),a2
	move.w	#510,d5
	; This first call primes all the register setup
	bsr	spi_rx_byte
	move.b	d0,(a2)+
rx_block:
	bsr	spi_rx_next
	move.b	d0,(a2)+
	dbra.w	d5,rx_block
	movem.l	(sp)+,a2/d2-d5
	rts

sd_spi_transmit_sector:
	movem.l	a2/d2-d5,-(sp)
	move.l	24(sp),a2
	move.w	#510,d5
	; This first call primes all the register setup
	move.b	(a2)+,d0
	bsr	spi_tx_byte
tx_block:
	move.b	(a2)+,d0
	bsr	spi_tx_next
	dbra.w	d5,tx_block
	movem.l	(sp)+,a2/d2-d5
	rts
