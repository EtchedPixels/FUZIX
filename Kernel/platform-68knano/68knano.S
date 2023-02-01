
#include "../kernel-68000.def"

		.globl plt_reboot
		.globl init_early
		.globl init_hardware
		.globl program_vectors
		.globl interrupt
		.globl plt_idle
		.globl outchar
		.globl plt_monitor

		.globl devide_read_data
		.globl devide_write_data

		.globl uartspi_rx
		.globl uartspi_tx
		.globl uartspi_rx_sector
		.globl uartspi_tx_sector

.mri 1
plt_monitor:
	; Need to check if top of memory at FE7000, if so
	; copy vectors from FE7000 to 0 (where we saved them)
	; them jmp to FE8000
plt_reboot:
	    or #$0700,sr
	    jmp start

init_early:
	    lea.l udata_block,a5	; udata ptr
	    move.l a5,udata_shadow	; shadow copy for entry/exit
            rts

plt_idle:
	    stop #($2000+EI_MASK)
	    rts
init_hardware:
	    move.w #1024,d0
	    move.w d0,ramsize
	    move.l #_end+512,d1
	    sub.l #_data,d1
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

;	IDE:
;
devide_read_data:
	move.l	d2,-(sp)
	move.l	blk_op,a0
	move.l	#$900000,a1
	move.w	#255,d0
	moveq	#8,d2
devide_read_l:
	move.w	(a1),d1
	rol.w	d2,d1
	move.w	d1,(a0)+
	dbra	d0,devide_read_l
	move.l	(sp)+,d2
	rts

devide_write_data:
	move.l	d2,-(sp)
	move.l	blk_op,a0
	move.l	#$900000,a1
	move.w	#255,d0
	moveq	#8,d2
devide_write_l:
	move.w	(a0)+,d1
	rol.w	d2,d1
	move.w	d1,(a1)
	dbra	d0,devide_write_l
	move.l	(sp)+,d2
	rts

;
;	Bitbangers for the SPI bus
;



; outchar: Wait for UART TX idle, then print the char in d0

outchar:
outcharw:
	; TODO
	rts
;


;
;	Receive a byte not caring about what we send (FF)
;
uartspi_rx_byte:
	movea.l	#$A00008,a0		; MCR
	lea.l	4(a0),a1		; MSR
	move.b	(a0),d1
	andb	#$FA,d1		; Clear inverted clock, data
	move.b	d1,d2
	addq.b	#$04,d2		; with CLK
	; Entry point if already have the precalc values
uartspi_rx_next:
	; Unrolled for speed for SD card use
	; Send 1 bits
	; Bit 0
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	; Bus should now hold our bit to sample
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 1
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 2
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 3
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 4
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 5
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 6
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	; Bit 7
	move.b	d2,(a0)			; With CLK low
	move.b	d1,(a0)			; With CLK high
	move.b	(a1),d3
	lsl.b	#1,d3
	roxl.b	#1,d0
	not.b	d0
	rts

uartspi_tx_byte:
	not.b	d0
	move.l	#$A00008,a0		; MCR
	lea.l	4(a0),a1		; MSR
uartspi_tx_next:
	move.b	(a0),d1
	and.b	#$FA,d1			; Clear inverted clock, data
	moveq	#7,d2
spi_tx_bit:
	lsr.b	#1,d1			; MCR right
	roxl.b	#1,d0			; Data bit
	roxl.b	#1,d1			; Into the MCR
	addq	#$04,d1			; Plus clock
	move.b	d1,(a0)
	subq	#$04,d1			; Minus clock
	move.b	d1,(a0)
	dbra.w	d2,spi_tx_bit
	rts

;
;	Wrappers for C calling
;
uartspi_rx:
	movem.l	d2-d3,-(sp)
	bsr	uartspi_rx_byte
	movem.l	(sp)+,d2-d3
	rts

uartspi_tx:
	movem.l	d2-d3,-(sp)
	move.w	12(sp),d0
	bsr	uartspi_tx_byte
	movem.l	(sp)+,d2-d3
	rts

;
;	For an SD card this is the most critical path
;
uartspi_rx_sector:
	movem.l	a2/d2-d4,-(sp)
	; Get the argument - address of buffer
	; As we are a flat system we don't have to worry about
	; kernel v user etc
	move.l	20(sp),a2
	move.w	#510,d4
	; This first call primes all the register setup
	bsr	uartspi_rx_byte
	move.b	d0,(a2)+
rx_block:
	bsr	uartspi_rx_next
	move.b	d0,(a2)+
	dbra.w	d4,rx_block
	movem.l	(sp)+,a2/d2-d4
	rts

uartspi_tx_sector:
	movem.l	a2/d2-d4,-(sp)
	move.l	20(sp),a2
	move.w	#510,d4
	; This first call primes all the register setup
	bsr	uartspi_tx_byte
	move.b	d0,(a2)+
tx_block:
	bsr	uartspi_tx_next
	move.b	d0,(a2)+
	dbra.w	d4,tx_block
	movem.l	(sp)+,a2/d2-d4
	rts
