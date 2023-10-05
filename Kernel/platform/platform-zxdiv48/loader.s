;
;	Some juggling required as we need to vapourise our environment
;	fairly early on but don't have a ton of safe working memory
;
;	We don't care too much how we get loaded as we just grab control
;	from whomever called us and own the system from that point.
;
;	We can run this code from BASIC, from some kind of DOS loader
;	etc, as a snapshot or just a tap loaded via FATware.
;

	.area _BOOT(ABS)
	.org 0xE000		; we are loaded at 0x2000 for ESX, but it
				; doesn't matter where in reality providing
				; it's not just below and overlapping E000

start:
	di
	ld sp,#0xE000
	ld hl,#0x2000
	ld de,#0xE000
	ld bc,#0x0400
	ldir
	jp go
;
;	We are now mapped as expected and in common high space
;
go:
	ld hl,#0x4000
	ld de,#0x4001
	ld bc,#0x1AFF
	ld (hl),#0
	ldir

	;
	;	Ensure the master drive is selected
	;
wait1:
	in a,(191)
	rla
	jr c, wait1
	ld a,#0xE0		; master in LBA
	out (187),a
	nop
wait2:
	in a,(191)
	and #0xC0
	cp #0x40		; want busy off, drdy
	jr nz, wait2

	;
	; Load sectors. We shortcut stuff here because we never
	; load over 256 sectors
	;
	; Sector 0 is the partition table in a PC style setup
	; Sectors 2048+ are the file system as a vaguely modern fdisk leaves
	; a nice big boot area which we will purloin
	;
	xor a			; LBA28 high bits
	out (179),a
	out (183),a

	out (254),a
	; 
	; DivIDE plus to RAM mode (this won't work on older DivIDE)
	;
	ld a,#0x60		; RAM mode bank 0, writable
	out (0x17),a

	ld de,#0x2001		; Load 32 sectors (0000-3FFF)
				; from sector 1 into bank 0
	ld hl,#0000		; Starting address to load
	call load_loop

	ld a,#0x01
	out (254),a

	ld a,#0x61
	out (0x17),a
	ld d,#0x20		; Load 32 sectors (0000-3FFF)
	ld hl,#0x0000		; into bank 1
	call load_loop

	ld a,#0x02
	out (254),a

	ld a,#0x62
	out (0x17),a
	ld d,#0x20		; Load 32 sectors (0000-3FFF)
	ld hl,#0x0000		; into bank 2
	call load_loop

	ld a,#0x03
	out (254),a

	ld a,#0x63
	out (0x17),a
	ld d,#0x20		; Load 32 sectors (0000-3FFF)
	ld hl,#0x0000		; into bank 3
	call load_loop

	ld a,#0x04
	out (254),a

	; FIXME: for this we should just load 5B00->7FFF
	ld d,#0x20		; Load 32 sectors (4000-7FFF)
	ld hl,#0x4000		; into 4000-7FFF
	call load_loop

	; FIXME: for this we should just load A000-BFFF
	ld d,#0x20		; Load 32 sectors (8000-BFFF)
	ld hl,#0x8000		; into 4000-7FFF
	call load_loop

	ld a,#0x05
	out (254),a

	ld a,#0x60		; switch back
	out (0x17),a
	xor a			; tell the kernel this is DivIDE plus mode
	jp 0x0080		; and into crt0.

load_loop:
	ld a,e
	inc e
	out (175),a		; sector number to load
	dec a

	; Progress bar
	push hl
	ld h,#0x58
	ld l,a
	ld (hl),#0x20		; green square
	pop hl

	ld a,#1			; load one sector
	out (171),a
	ld a,#0x20		; READ
	out (191),a
	nop
wait3:
	in a, (191)
	rlca
	jr c, wait3		; Busy
	bit 4,a			; DRQ ?
	jr z, failed		; Nope - bad
	ld bc,#163		; Data port
	inir
	inir
	dec d
	jr nz, load_loop

	ret

failed:
	xor a
	out (254),a
	di
	halt
