/*
 *	The first 512 bytes are ROM loaded, and we are passed helper
 *	addresses to run the rest of the load
 *
 *	TODO: save TRAP14 vector so we can put it back later
 */
	#include "../../cpu-68000/kernel-68000.def"

	.globl __end
	.globl __bss_start
	.globl start

.mri 1
loader:
	; Start loading blocks. a2 is currently $600 and is preserved and
	; updated by the runrd (a5) call.
	mov.w #$7F,d7		; Load 64K
nblock:
	add.b #1,blow+1		; move on an LBA block (max 255)
	lea rdlba(pc),a1
	jsr (a6)
	jsr (a5)
	
	mov.b #$2A,$FF806

	dbra.w d7,nblock
	bra start
rdlba:
	byte $F2,$01		; sector count
blow:	byte $F3,$01		; we inc first so start at block 2
	byte $F4,$00
	byte $F5,$00
	byte $F7,$20
	byte $00,$00

start:
	lea.l udata_block+UBLOCK_SIZE,a7
	or #$0700,sr

	mov.b #13,d0
	bsr outchar
	mov.b #10,d0
	bsr outchar

	move.l #__bss_start,a0
	move.l #__end,d0
	sub.l a0,d0
	lsr.l #2,d0
wipebss:
	clr.l (a0)+
	dbra d0,wipebss

	bsr init_early
	bsr init_hardware
	bsr fuzix_main
	or #$0700,sr
stop:	bra stop
