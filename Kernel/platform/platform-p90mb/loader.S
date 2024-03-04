/*
 *	We are loaded somewhere out of the memory window and run
 *	Our stack is a C call stack of the form
 *
 *	bootloader(uint8_t bootdev, uint8_t *base, uint32_t len, syscallvec)
 *
 *	Needs to bne relocatable.
 */
	.globl start
	.globl loader

.mri	1

/* We load the binary into the memory space RetroDOS provided. We will then
   relocate it once we are ready to dump RetroDOS overboard */

	ascii "RETRODOSBOOT68"
loader:
	lea.l syscb(pc),a0
	move.l 4(sp),4(a0)		; drive
	move.l 8(sp),8(a0)		; load address

	move.l 16(sp),a4		; syscall vector

	move.l a0,-(sp)
	jsr (a4)			; C call into disk loader
	addq #4,sp

/* Now dump RetroDOS and run it. This assumes we move the image down which
   happens to be a safe bet but we ought to check and do either direction */

	move.l 8(sp),a0
	move.l #$0400,a1
	move.w #$3FFF,d0		; 64K

copy:
	move.l (a0)+,(a1)+
	dbra d0,copy

	jmp $0400

syscb:
	long $F1			; raw read
cbdrive:
	long 0				; drive
cbaddr:
	long 0				; buffer
	long 1				; LBA
	long 128			; length	 (64K for now)
