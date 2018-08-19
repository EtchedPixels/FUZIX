;
;	Boot block, loaded at 0x8000 in the top bank that is free (varies)
;	but this works so long as we are above bank 7. If that becomes
;	an issue then instead of flipping banks we can always ldir ourself
;	into bank 0 and run from there.
;
;	The boot block is trivial but it gets a bit fun once we've loaded
;	our 14 tracks
;
;	We load
;	32K into bank 2/3
;	32K into bank 4/5
;	5.5K into bank 6/7
;
;	by loading 14 tracks off side 0 skipping track 0 sector 1 which is
;	the boot block itself.
;
;	We then run from 0x8000 in bank 6, which will untangle everything
;	for us shuffling the kernel so that 0/1 is the low kernel in RAM
;	2/3 is the high kernel in RAM and 4/5 is the video
;
	.area BOOT(ABS)
	.org 0x0000

HIMEM	.equ	251
LOMEM	.equ	250
STATUS	.equ	224
CMD	.equ	224
TRACK	.equ	225
SECTOR	.equ	226
DATA	.equ	227

CMD_READ .equ	0x80
CMD_STEPIN .equ 0x58

	.db 0,0,0,0,0,0,0,0,0,0	; need to work out what goes here
boot:
	di
	; Move ourself from 0x8000 to 0x0000
	in a,(HIMEM)
	and #0x1F
	or #0x20
	out (LOMEM),a
	; Now move to the low mem copy
	jp go
go:
	ld sp,#go
	ld de,#0x0402		; track 4 sector 2 (we are sector 1)
	ld a,e			; Start in bank 2
	ld hl,#0x8000		; Which we map high
	out (HIMEM),a
	ex af,af'
next_sec:
	bit 7,h			; If the last read went to 0000
	jr nz, still_good
	set 7,h			; Go back to 8000 and move on 32K
	ex af,af'
	inc a
	inc a
	out (HIMEM),a		; switch bank
	ex af,af'
still_good:
	ld a,e			; update the sector register
	out (SECTOR),a
dread:	ld a,#CMD_READ		; Ask for data from the FDC
	out (CMD),a
	ld b,#20		; Wait for FDC
nap:	djnz nap
	ld bc,#DATA		; Begin reading from data port
	jr wait
byte:	ini
wait:	in a,(STATUS)		; Wait for DRQ
	bit 1,a
	jr nz,byte		; When we get a DRQ read a byte
	rrca			; Check for end/error
	jr c,wait
	; and with 0d to check error FIXME
	;
	;	Sector done. Move on a sector
	;
	inc e
	ld a,#11
	cp e
	jr nz, next_sec
	;
	;	Move on a track - do a step in command
	;
	ld e,#1
	inc d
	;
	;	We read 14 tracks minus 1 sector which loads us
	;
	;
	ld a,#18
	cp d
	; Jump to the start of the last bank loaded. This is actually
	; packed up boot stuff and font. We loaded 5.5K here, of which 4K
	; was font, leaving a spacious 1.5K to untangle all the banks and
	; boot up.
	jp z, 0x8000
	ld a,#CMD_STEPIN
	out (CMD),a
	ld b,#20
nap2:	djnz nap2
wait2:	in a,(STATUS)
	bit 0,a
	jr nz,wait2
	jr next_sec

	.org 0x0100
	.ascii 'BOO'
	.byte 'T'+0x80
