
	.abs

	.org 0x0000

start:
	di
	ld sp, 0x0000		; safe spot
	ld a, 0x80
	out (0x1f),a		; ROM out
	xor a
	out (0x1f),a
	ld hl, 0xB000
	ld de, 0x0000
	ld bc, 0x0100
	ldir
	jp go
go:
	; Init the ATA CF
	ld a, 0xE0
	out (0x16),a
	xor a
	out (0x14),a
	out (0x15),a
	; Set 8bit mode
	call wait_ready
	ld a, 1			; 8bit PIO
	out (0x11),a
	ld a, 0xEF		; SET FEATURES (8bit PIO)
	out (0x17),a
	call wait_ready

	; Load the kernel
	ld d, 1
	ld bc, 0x10		; c = data port  b = 0
	ld hl, 0x0100		; Load 0x0100-F000 (should be sufficient)
loader:
	inc d
	call load_sector
	ld a, 0xF1
	cp h
	jr nz, loader

	ld hl,(0x0103)
	ld de, 0x10DE
	or a
	sbc hl,de
	jr nz, badimage

	jp 0x0100
badimage:
	di
	halt
;
;	Load sector d from disk into HL and advance HL accordingly
;
load_sector:
	ld a,d
	out (0x13),a		; LBA
	ld a, 1
	out (0x12),a		; 1 sector
	ld a, 0x20
	out (0x17),a		; command
	; Wait 
wait_drq:
	in a,(0x17)
	bit 3,a
	jr z, wait_drq
	; Get data, leave HL pointing to next byte, leaves B as 0 again
	inir
	inir
	ret
wait_ready:
	in a,(0x17)
	bit 6,a
	jr z,wait_ready
	ret
