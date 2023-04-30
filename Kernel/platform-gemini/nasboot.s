;
;	NASCOM boot block
;
;	The ROM loads 512 bytes from track 0, side 0, sector 1 into a buffer
;	and then down to 0x0100
;
;	The block must start with the code 'NCB' and the next byte is the
;	entry point.
;
;	The stack is below 0100 and the ROM is fixed at F000
;
;	It has a jump table at the start with helper routines
;
;	Our disk is selected
;
;	FC74/5/6 hold the disk, track, sector
;
;	track 0-76 - side 0, 77+ side 1!
;
;	This will not work on other controllers. The MAP80 for example
;	doesn't have the same ROM arrangement and loads sector 0 into
;	0x0C00, needs 8080 at the start then jumps to n + 2
;
	.area ASEG(ABS)
	.org 0x0100
	.byte 'N'
	.byte 'C'
	.byte 'B'

boot:
	ld hl,#0x0200		; Load address of image
diskloop:
	ld a,(0xfc76)		; sector 0-9
	inc a
	cp #10
	call nz, newtrack
	ld (0xfc76),a
	push hl
	call 0xf43d		; read block
	pop hl
	inc h
	inc h
	ld a,#0xE8
	cp h
	jr nz, diskloop
	jp 0x200
newtrack:
	ld a,(0xfc75)
	inc a
	ld (0xfc75),a
	xor a
	ret

	