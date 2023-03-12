/*
 *	BOOT ROM. We use a custom ROM for this as we need to forward
 *	vectords and stuff
 */

		/* Just for debugging */

		.globl	start
		.globl	load_one
		.globl	waitdata
		.globl	setlba
		.globl	sd_init
		.globl	sendacmd
		.globl	sendcmd
		.globl	waitff
		.globl	waitret
		.globl	cslower
		.globl	csraise
		.globl	sendff
		.globl	txbyte
		.globl	get4
		.globl	outstr
.mri 1

vectors:
		long	$2FFF0		; Assumes 128K
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
		long	e_spurious
		long	e_unexpected
		long	e_unexpected
		long	e_unexpected
		long	e_interrupt			; VIA and serial
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
		long	e_trap12			; System call
		long	e_trap13
		long	e_trap14
		long	e_trap15

spurious	equ	$10008
unexpected	equ	$10008+6
bus		equ	$10008+12
addr		equ	$10008+18
illegal		equ	$10008+24
divzero		equ	$10008+30
chk		equ	$10008+36
trapv		equ	$10008+42
priv		equ	$10008+48
trace		equ	$10008+54
unimpa		equ	$10008+60
unimpf		equ	$10008+66
uninit		equ	$10008+72
interrupt	equ	$10008+78
trap0		equ	$10008+84
trap1		equ	$10008+90
trap2		equ	$10008+96
trap3		equ	$10008+102
trap4		equ	$10008+108
trap5		equ	$10008+114
trap6		equ	$10008+120
trap7		equ	$10008+126
trap8		equ	$10008+132
trap9		equ	$10008+138
trap10		equ	$10008+144
trap11		equ	$10008+150
trap12		equ	$10008+156
trap13		equ	$10008+162
trap14		equ	$10008+168
trap15		equ	$10008+174

e_bus_error:
		jmp	(bus)
e_addr_error:
		jmp	(addr)
e_illegal:
		jmp	(illegal)
e_divzero:
		jmp	(divzero)
e_chk:
		jmp	(chk)
e_trapv:
		jmp	(trapv)
e_priv:
		jmp	(priv)
e_trace:
		jmp	(trace)
e_unimpa:
		jmp	(unimpa)
e_unimpf:
		jmp	(unimpf)
e_uninit:
		jmp	(uninit)
e_interrupt:
		jmp	(interrupt)
e_trap0:
		jmp	(trap0)
e_trap1:
		jmp	(trap1)
e_trap2:
		jmp	(trap2)
e_trap3:
		jmp	(trap3)
e_trap4:
		jmp	(trap4)
e_trap5:
		jmp	(trap5)
e_trap6:
		jmp	(trap6)
e_trap7:
		jmp	(trap7)
e_trap8:
		jmp	(trap8)
e_trap9:
		jmp	(trap9)
e_trap10:
		jmp	(trap10)
e_trap11:
		jmp	(trap11)
e_trap12:
		jmp	(trap12)
e_trap13:
		jmp	(trap13)
e_trap14:
		jmp	(trap14)
e_trap15:
		jmp	(trap15)
e_spurious:
		jmp	(spurious)
e_unexpected:
		jmp	(unexpected)

start:
		lea	$30000,a6
		; Copy the read command template into RAM
		move.l	cmd17(pc),-8(a6)
		move.l	cmd17+4(pc),-4(a6)
		; ACIA setup
		move.b	#$96,1(a6)
		lea	banner(pc),a0
		bsr	outstr

		moveq	#7,d0
		move.b	d0,6(a6)		; CS CLK MOSI output
		move.b	d0,2(a6)			; CS CLK MOSI high

		bsr	sd_init

		tst.b	d0
		bne	sd_ok
		lea	nosd(pc),a0
		bsr	outstr
		bra	stop

sd_ok:
		move.b	d0,-2(a6)		; Top word of RAM is SD type

		lea	sdgood(pc),a0
		bsr	outstr

		lea	$10000,a0
		moveq	#1,d7			; Load blocks after partition table
load_one:
		move.b	#$2E,d0
		bsr	outch
		lea	-8(a6),a1		; Read command copy
		bsr	setlba
		bsr	sendcmd
waitdata:
		bsr	sendff
		cmp.b	#$FE,d0
		bne	waitdata
		move.w	#511,d1
dataloop:
		bsr	sendff
		move.b	d0,(a0)+
		dbra.w	d1,dataloop
		bsr	csraise
		addq	#1,d7
		cmp.b	#128,d7			; 63.5K should be tons
		; Note that if you go over block 127 you need to modify
		; setlba too
		bne	load_one
		lea	newline(pc),a0
		bsr	outstr
		lea	$10000,a0
		cmp.l	#$AA556800,(a0)+
		bne	bad_boot
		move.l	(a0),a0
		jmp	(a0)
bad_boot:
		lea	bad(pc),a0
		bsr	outstr
stop:		bra	stop

setlba:
		btst.b	#6,-2(a6)		; SD type in top word of RAM
						; bit 6 set for block mode
		beq	byte_offset
		move.b	d7,4(a1)
		rts
byte_offset:
		; We load below LBA 128 so we can stick
		; to simple setup of blocks
		move.b	d7,d0
		add.b	d0,d0
		move.b	d0,3(a1)	; Set the LBA to 512 x offset
		rts

sd_init:	move.w	#31,d1
ffclock:
		bsr	sendff
		dbra	d1,ffclock
		lea	cmd0(pc),a1
		bsr	sendcmd
		cmp.b	#1,d0
		bne	not_sd
		lea	cmd8(pc),a1
		bsr	sendcmd
		cmp.b	#1,d0
		bne	old_card
		bsr	get4		; bufptr back in a0
		cmp.w	#$01AA,2(a0)
		bne	not_sd
wait41:
		lea	acmd41(pc),a1
		bsr	sendacmd
		bne	wait41
		lea	cmd58(pc),a1
		bsr	sendcmd
		bne	not_sd
		bsr	get4
		btst.b	#6,(a0)
		move.b	#$80,d0	; SD 2 block oriented
		bne	is_block
		moveq	#$02,d0	; SD 2
is_block:
		rts
not_sd:
		clr.b	d0
		rts
old_card:
		lea	acmd41_0(pc),a1
		bsr	sendacmd
		cmp.b	#2,d0
		bcc	is_mmc
wait41_0:
		lea	acmd41_0(pc),a1
		bsr	sendacmd
		bne	wait41_0
		bsr	secsize
		moveq	#$01,d0	; SD 1
		rts
is_mmc:
		lea	cmd1(pc),a1
		bsr	sendcmd
		bne	is_mmc
		bsr	secsize
		moveq	#$03,d0	; MMC
		rts
secsize:
		lea	cmd16(pc),a1
		bra	sendcmd

sendacmd:
		move.l	a1,a2
		lea	cmd55(pc),a1
		bsr	sendcmd
		move.l	a2,a1
sendcmd:
		bsr	csraise
		bsr	cslower
		moveq	#5,d1
		cmp.b	#$40,(a1)
		beq	cmdout
waitff:
		bsr	sendff
		cmp.b	#$ff,d0
		bne	waitff
cmdout:
		move.b	(a1)+,d0
		bsr	txbyte
		dbra.w	d1,cmdout
		bsr	sendff
waitret:
		bsr	sendff
		btst.b	#7,d0
		bne	waitret
		tst.b	d0
		rts

cslower:
		move.b	#3,2(a6)
		rts
csraise:
		move.b	#7,2(a6)	; CS CLK DATA high

;
;	Optimised send ff read back
;
sendff:
		lea	2(a6),a5
		move.b	(a5),d2
		or.b	#3,d2		; clock high
		move.b	d2,d3
		and.b	#5,d3		; clock low
		moveq	#1,d4
		; bit 0
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 1
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 2
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 3
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 4
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 5
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 6
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		; bit 7
		move.b	d3,(a5)
		move.b	d2,(a5)
		move.b	(a5),d5
		roxl.b	d4,d5
		roxl.b	d4,d0
		rts

		; D0 input , preserve D1/7, return value in D0
		; uses most of the D regs
txbyte:
		move.b	2(a6),d2
		or.b	#3,d2		; CLK DATA high, copy CS
		moveq	#7,d3		; 8bits
sendbit:
		lsr.b	#1,d2		; working reg state
		roxl.b	#1,d0		; bit to send to C
		roxl.b	#1,d2		; bit to send into low D2
		subq	#2,d2
		move.b	d2,2(a6)	; clock low
		addq	#2,d2
		move.b	d2,2(a6)	; high
		move.b	2(a6),d4
		roxl.b	#1,d4
		roxl.b	#1,d5
		dbra.w	d3,sendbit
		move.b	d5,d0
		rts

get4:
		moveq	#3,d1
		lea.l	$10000,a0
getl:
		bsr	sendff
		move.b	d0,(a0)+
		dbra.w	d1,getl
		lea.l	$10000,a0
		rts

outstr:
		move.b	(a0)+,d0
		beq	outsd
		bsr	outch
		bra	outstr
outsd:		rts

outch:		btst.b	#1,1(a6)
		beq	outch
		move.b	d0,3(a6)
		rts

;
;	Commands
;
cmd0:
	byte $40,$00,$00,$00,$00,$95
cmd1:
	byte $41,$00,$00,$00,$00,$01
cmd8:
	byte $48,$00,$00,$01,$AA,$87
cmd16:
	byte $50,$00,$00,$02,$00,$01
cmd55:	
	byte $77,$00,$00,$00,$00,$01
cmd58:
	byte $7A,$00,$00,$00,$00,$01
acmd41_0:
	byte $69,$00,$00,$00,$00,$01
acmd41:
	byte $69,$40,$00,$00,$00,$01
cmd17:
	byte $51,$00,$00,$00,$00,$01

banner:
	ascii "Pico68 SDBoot"
	byte 13,10,0

	even
bad:
	ascii "Not bootable"
	byte 13,10,0

	even
sdgood:
	ascii "SD card detected"
	byte 13,10,0

	even
nosd:
	ascii "No SD card"
newline:
	byte 13,10,0
