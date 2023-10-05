;
;	The bootstrap loads a chunk of sectors from disk. In the ubee
;	case it's easier than many as we have a bootrom image at 0xE000
;	until we blow it away
;
;	FIXME: do any platforms need us to force RAM banks (eg so top and
;	lower RAM are not both the same bank), or unmap other ROM here
;
;	Preserve EFFx for the loaded image (so it can spot a 256TC)
;

	.area ASEG(ABS)
	.org 0x80

boot:	jr bootme
	.word 0xAA55		; signature
bootme:
	ld hl,#0x200
	ld de,#0x0002		; track 0 sector 2
	ld bc,#0xDC00		; D800 takes us up to DEFF, and DFxx is
				; ROM owned. We'll use DFxx+ for BSS etc
	call 0xE039		; ROM request
	jr z, allgood		; Loaded
	ld hl,#loadfail
fail:
	call 0xE033		; Flash up error
	jp 0xE003		; Monitor
loadfail:
	.ascii 'Load failed'
	.byte 0x80
notbootable:
	.ascii 'Not bootable'
	.byte 0x80
allgood:
	ld hl,(0x200)		; but is it crap ?
	ld de,#0xC0DE
	or a
	sbc hl,de
	ld hl,#notbootable
	jr nz,fail
	jp 0x0202
	