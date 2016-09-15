;
;	Boot blocks for UZI+ on the NC100
;
;
;	FIXME: we need a valid NMI vector in the low 0x100 at all times
;

;
;	These must match the actual nc100.s code
;
entry_so	.equ 0x1BE
kernel_sp	.equ 0x1C0
resume_vector	.equ 0x1C2
suspend_map	.equ 0x1C4
entry_banks	.equ 0x1C8
suspend_stack	.equ 0x1FE

		.org 0xE0
;
;	Stub to switch into the resume bank and code
;
		out (0x13),a
		jp (hl)
;
;	We are assembled for 0x100 but actually load at 0xC000 initially
;	but will relocate ourselves.
;
		.org 0x100
		di
	        ld (entry_sp),sp
                ld hl, (0xB000)	; Save MMU bank copies from the OS
	        ld (entry_banks),hl
	        ld hl, (0xB002)
	        ld (entry_banks+2),hl
		;
		;	Resume or new ?
		;
	        ld hl,(resume_vector)
	        ld de,#0
	        ld (resume_vector),de
	        ld a,h
	        or l
	        jr z, no_resume
;
;	We want to put our suspend page back at 0xC000 but we can't do that
;	directly as we are right now executing that bank
;
	        ld a,(suspend_map + 3)
	        jp 0xE0		; stub to bank switch and jp

no_resume:
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
