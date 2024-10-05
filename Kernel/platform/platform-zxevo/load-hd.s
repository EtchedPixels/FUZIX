;
;	Load the kernel from sectors 3+ of the media
;

	.area _BOOT(ABS)

	.org 0x6000

start:
	di
	ld sp,#0x6000

	; Wipe the video memory so we have something to show we live
	ld hl,#0x4000
	ld de,#0x4001
	ld bc,#0x1800
	ld (hl),#0
	ldir
	ld bc,#0x2ff
	ld (hl),#7
	ldir
	ld a,#7
	out (0xFE),a
	; Shadow on
	ld bc,#0xBF
	ld a,#1
	out (c),a

	; Set the MMU up so that our code is MMU mapped to match the
	; direct mapping
	ld bc,#0x77f7
	ld a,#0x5
	cpl
	out (c),a

	ld bc,#0xF177	;	ROM defaults, with MMU on
	ld a,#0x03	;	spectrum, no turbo
	out (c),a


	; Kernel lives in 0 / 1 / 2 / 3

	ld e, #3			; sector
	xor a
	call load_bank
	ld a,#1
	call load_bank
	ld a,#2
	call load_bank
	ld a,#3
	call load_bank

	; Now put the MMU right
	ld bc,#0x37f7
	ld a,#0xff
	out (c),a
;	ld b,#0x77		; Skip setting this page because
	dec a			; we are running in it
;	out (c),a
	ld b,#0xC7
	dec a
	out (c),a
	ld b,#0xF7
	dec a
	out (c),a
	; And go (we can't map everything yet but the code at 0x0100 will do
	; the final mapping). Note we enter with shadow on (for the final
	; MMU call)
	ld b,#0x77
	ld a,#1
	out (0xfe),a
	ld a,#0xfe
	jp 0x0100

;
;	Load bank A
;
load_bank:
	ld bc,#0xf7f7
	cpl
	out (c),a
	ld b,#32
	ld hl,#0xC000
load_bank_loop:
	call sector
	djnz load_bank_loop
	ret

sector:
	ld a,e
	inc e
	out (0x70), a
	ld a,#1
	out (0x50), a
	ld a,#0x20
	out (0xF0), a
	nop
	nop
w0:
	in a,(0xF0)
	rla
	jr c, w0
	and #0x10		; DRQ shifted
	jr nc, w0

	ld bc,#0x10
	inir
	inir
w1:
	in a,(0xF0)
	rla
	jr c, w1
	rla
	jr nc, w1

	ret
