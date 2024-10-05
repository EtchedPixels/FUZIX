;
;	Initial test loader for TRS80 hard disk booting. This assumes
;	- 32 sectors per track (8K) and at least 8 heads. More spt is easy
;	  to do, but if you can't get 64K into track 0 then the code will
;	need testing further
;

		.z80

SDH = 0x80		; SDH upper bit settings (drive 1)
HEADS = 8		; Heads on drive
SPT = 32		; Sectors per track


		.area BOOT(ABS)
		.org 0x0000

start:
	ld a,#0x18
	out (0xC1),a		; soft reset the HDC (takes time so start now)
	ld a, #0x86			; kernel map, 80 column, no remap
	out (0x84), a		; video page 1
	ld a, #0x50			; 80 column, sound, altchars off,
					; ext I/O on , 4MHz
	out (0xEC), a
	ld hl, #0x4300
	ld de, #0x0
	ld bc, #256
	ldir
	jp go

go:
	ld sp,#0xEE00

        ; load the 6845 parameters
	ld hl, #_ctc6845		; reverse order
	ld bc, #0x0F88
ctcloop:
	out (c), b			; select register
	ld a, (hl)
	out (0x89), a			; output data
	dec hl
	djnz ctcloop

	; clear screen
	ld hl, #0xF800
	ld de, #0xF801
	ld bc, #0x07FF
	ld (hl), #' '
	ldir
	ld ix, #0xF800
	ld (ix), #'H'

	; Finish set up
	ld a,#0x0C
	out (0xC0),a
	ex (sp),hl
	ex (sp),hl
	; Should now go ready on all controller types

	; Issue a restore 4ms command
waitrdy:
	in a,(0xCF)
	and #0x82
	jr nz, waitrdy

	ld 1(ix), #'D'

	ld hl, #0x0100


	; Load cylinder 0 head 0 except sector 1-2 onwards

	; Set the drive and sector
	; drive bits are:
	; no ECC bytes, 256 bytes per sector, (need to check ECC one)
	ld d,#SDH
	ld e,#0x02

	; Don't set the registers until the restore is done
waitrdyc:
	in a,(0xCF);
	and #0x80
	jr nz, waitrdyc

	ld 2(ix), #':'
	; Set the cylinder number
	xor a
	out (0xCC),a	; cylinder 0
	out (0xCD),a

nexttrack:
	ld b,a
	ld c,a
	push bc
	ld bc,#0x00C8	; data port, 256

nexthead:
nextsec:
	; Everything loaded ?
	ld a,#0xEE
	cp h
	jp z,0x0100
	ld 3(ix),#0x91
	; Set up the request (cylinder is already 0)
	ld a,d		; SDH
	out (0xCE),a
	ld a,e		; Sector
	out (0xCB),a
	ld a,#1
	out (0xCA),a	; Count
rwait:
	in a,(0xCF)
	and #0x80
	jr nz,rwait
	ld 3(ix),#0x95
	ld a,#0x20	; Read sector
	out (0xCF),a
	; Read now running
dwait:
	in a,(0xCF)
	and #0x08
	jr z, dwait

	ld 3(ix),#0xB7

	; We have data
	inir

	ld 3(ix),#0xBF

	inc ix

	inc e
	ld a,#SPT
	cp e
	jr nz, nextsec
	ld e,#0
	inc d
	ld a,#HEADS|SDH
	cp d
	jr nz, nexthead
	pop bc
	inc bc
	ld a,c
	out (0xCC),a
	ld a,b
	out (0xCD),a
	ld d,#SDH
	jr nexttrack

				; ctc6845 registers in reverse order
	    .db 99
	    .db 80
	    .db 85
	    .db 10
	    .db 25
	    .db 4
	    .db 24
	    .db 24
	    .db 0
	    .db 9
	    .db 101
	    .db 9
	    .db 0
	    .db 0
	    .db 0
_ctc6845:   .db 0
