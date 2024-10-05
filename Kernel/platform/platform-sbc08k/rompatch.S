/*
 *	We have 256 bytes to load the boot sector from IDE!
 *
 *	This gets built and patched into the free bytes at the end of
 *	the standard ROM. There are 256 bytes free at E3F00)
 *
 *	To boot just run it from the ROM.
 *	It loads LBA 1 (sector 2) for 256 bytes in 16bit mode (so data must
 *	be doubled). This loads the secondary bootstrap which then uses this
 *	code to switch to 8bit mode and load LBA 2 onwards into memory from
 *	$0500 for 64K then runs $0500.
 *
 *	A0 - points to PI/T + 18
 *	A1 - points to comamand in write regs
 *	A2 - address for A5 (updated and preserved)
 *	A3 - points to PI/T + 16
 *	A4 - point sto PI/T + 4
 *	A5 - read block routine - reads 256 bytes
 *	A6 - write regs routine (and thus sort of run command)
 */
	.mri 1

load:
	lea $3FE,a7		; Scribble over upper vectors
	lea $FFFF000,a0		; PI/T
	lea 16(a0),a3		; PI/T PADR
	lea 4(a0),a4		; PI/T PADDR
	clr.b (a0)
	clr.b (a4)
	mov.b #$FF,6(a0)
	mov.b #$40,12(a0)
	mov.b #$40,14(a0)
	add.w #18,a0		; PI/T PBDR
	lea runcmd(pc),a6	; For bootstrap to re-use
	lea runrd(pc),a5
	lea go512(pc),a1
	bsr runcmd
	mov.w #$0400,a2
	lea setup(pc),a1
	bsr runcmd
	bsr runrd
	jmp $0400
runcmd:
wrdy:
	mov.b #$F7,(a0)		; CS for status
	mov.b #$B7,(a0)
	btst #7,(a3)
	bne wrdy
	bra dowr
nextw:
	mov.b (a1)+,d1
	beq dorts
	mov.b (a1)+,(a3)
	mov.b d1,(a0)
	mov.b #$FF,(a4)		; write direction
	and.b #$DF,d1		; drop WR
	mov.b d1,(a0)
	or.b #$20,d1
	mov.b d1,(a0)
dowr:
	mov.b #$F8,(a0)		; WR and CS back high
	clr.b (a4)		; Back to read
	bra nextw

runrd:
wdrq:
	mov.b #$F7,(a0)		; CS for status
	mov.b #$B7,(a0)
	btst #3,(a3)
	beq wdrq
	mov.b #$F8,(a0)
	mov.w #$1FF,d0		; 512 bytes
	mov.b #$F0,(a0)
nextr:
	mov.b #$B0,(a0)
	mov.b (a3),(a2)+
	mov.b #$F0,(a0)
	dbra.w d0,nextr
	; 512 bytes loaded
dorts:	rts

setup:
	byte $F6,$E0		; LBA mode
	byte $F2,$01		; sector count
	byte $F3,$01
	byte $F4,$00
	byte $F5,$00
	byte $F7,$20		; read LBA 2(second sector)
	byte $00
go512:
	byte $F1,$01
	byte $F7,$EF
	byte $00

	even
