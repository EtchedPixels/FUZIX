/*
 *	Boot ROM
 */

.mri 1
	long	$07FFFFF8
	long	start
	long	e_bus_error
	long	e_addr_error
	long	e_illegal
	long	e_divzero
	long	e_chk
	long	e_trapv
	long	e_priv
	long	e_trace
	long	e_unimpa
	long	e_unimpf
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_uninit
	; 16-23 unused
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	; 24
	long	timer
	long	interrupt
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	long	e_unexpected
	; 32 - TRAP #n
	long	e_trap0
	long	e_trap1
	long	e_trap2
	long	e_trap3
	long	e_trap4
	long	e_trap5
	long	e_trap6
	long	e_trap7
	long	e_trap8
	long	e_trap9
	long	e_trap10
	long	e_trap11
	long	e_trap12
	long	e_trap13
	long	e_trap14
	long	e_trap15

e_bus_error:
e_addr_error:
e_illegal:
e_divzero:
e_chk:
e_trapv:
e_priv:
e_trace:
e_unimpa:
e_unimpf:
e_unexpected:
e_uninit:
e_trap0:
e_trap1:
e_trap2:
e_trap3:
e_trap4:
e_trap5:
e_trap6:
e_trap7:
e_trap8:
e_trap9:
e_trap10:
e_trap11:
e_trap12:
e_trap13:
e_trap14:
e_trap15:
halt:
	bra	halt

chout:
	btst	#$5,5(a5)
	beq	chout
	move.b	d2,(a5)
done:
	rts

strout:
	move.b	(a6)+,d2
	beq	done
	bsr	chout
	bra	strout

timer:
	rte
interrupt:
	rte

start:
	move.w	#$2700,sr
	move.w	#7,d1
	movea.l	#$04000000,a0
	movea.l	a0,a2
initdram:
	move.l	(a0),d0
	nop
	dbra.w	d1,initdram


	movea.w	#$FFFFF0C0,a5

	move.b	#$80,3(a5)
	move.w	#$0100,(a5)
	move.b	#$3,3(a5)
	clr.b	2(a5)
	move.b	#$03,4(a5)
	clr.b	1(a5)

	lea	banner(pc),a6
	bsr	strout

	movea.w	#0,a1
	move.w	#$8000,d0

	; Now load the binary

	
	movea.w	#$FFFFF010,a1
	move.b	#$E0,6(a1)
	bsr	waitrdy
	move.b	#$01,1(a1)		; features 1
	move.b	#$EF,7(a1)		; set features 8bit
	bsr	waitrdy

	clr.w	4(a1)			; clear lba upper
	moveq	#1,d0
	move.b	d0,2(a1)		; sector count

;	A0 is still set the the base of RAM at the moment

	lea	loading(pc),a6
	bsr	strout

	movb	#$2E,d2
;
;	Copy loop
;
next_sector:
	move.b	d0,3(a1)		; set LBA 1-255 as we go
	move.b	#$20,7(a1)		; command READ
	bsr	waitdrq

	move.w	#511,d1
sector_in:
	move.b	(a1),(a0)+
	dbra.w	d1,sector_in
	bsr	waitrdy

	bsr	chout

	addq.b	#1,d0			; we load 128K
	bne	next_sector

	lea	running(pc),a6
	bsr	strout

	move.l	4(a2),a0		; from app header

	;	The JSR will push the old PC to -(SP) aka
	;	FFFF8000 which will in turn flip the memory
	;	bank before we arrive. We use (a0) to ensure
	;	everything is fetched
	movea.w	#$FFFF8004,a7
	jsr	(a0)
	;	Never returns
	bra	halt

; Should do error handling off bit 0 if not busy and error set
waitrdy:
	btst.b  #$7,7(a1)
	bne	waitrdy
	btst.b	#$6,7(a1)
	beq	waitrdy
	rts
waitdrq:
	btst.b	#$3,7(a1)
	beq	waitdrq
	rts

banner:
	ascii	"MB020 ROM Bootstrap"
	byte	13,10
	byte	0
loading:
	ascii	"Loading: "
	byte	0
running:
	byte	13,10
	ascii	"Booting."
	byte	13,10,0
