;
;	Boot blocks for UZI+ on the NC100
;
;
;	FIXME: we need a valid NMI vector in the low 0x100 at all times
;

		.org 0x100
		di
		ld a, 0x83	; map the low 16K of the kernel
	        out (0x10), a
		ld hl, 0xC000	; copy ourself into the low 16K
		ld de, 0x0000
		ld bc, 0x4000
		ldir
		ld a, 0x84
		out (0x11), a
		ld a, 0x81
		out (0x12), a
		ld hl, 0x8000
		ld de, 0x4000
		ld bc, 0x4000
		ldir
		ld a, 0x85
		out (0x11), a
		ld a, 0x82
		out (0x12), a
		ld hl, 0x8000
		ld de, 0x4000
		ld bc, 0x4000
		ldir
		ld a, 0x84	; map the other 32K of the kernel
		out (0x11), a
		ld a, 0x85
		out (0x12), a
		ld a, 0x86
		jp switch	; get out of the segment that is going to vanish
switch:		out (0x13), a	; map the common
		jp 0x0213	; into crt0.s

		.org 0x200
;
;	We should hide a logo in here ...
;
signature:	.db     "NC100PRG"
padding2:	.db	0,0,0,0,0,0,0,0

;
;	At this point we are mapped at 0xC000 so this code is running from
;	0xC210 in truth. Only at the jp to switch do we end up mapped low
;
start:		jp 0xC100
;
;	Drops into the copy of the image
;
