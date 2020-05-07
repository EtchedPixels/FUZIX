	.area _BOOT(ABS)

	.org 0x0100

start:
	di
	ld sp,#0x0000		; safe spot
	ld hl,#signon
	call outstr
	ld a,#0x11
	out (0x1f),a
	ld hl,#0xB000
	ld de,#0x0100
	ld b,d
	ld c,e
	ldir
	jp go
go:
	; Init the ATA CF
	ld a,#0xE0
	out (0x16),a
	xor a
	out (0x14),a
	out (0x15),a
	; Set 8bit mode
	call wait_ready
	ld a, #1		; 8bit PIO
	out (0x11),a
	ld a, #0xEF		; SET FEATURES (8bit PIO)
	out (0x17),a
	call wait_ready

	; Load the kernel
	ld d,#1
	ld bc,#0x10		; c = data port  b = 0
	ld hl,#0x1000		; Load 1000-D000 (should be sufficient)
loader:
	inc d
	call load_sector
	ld a,#0xD0
	cp h
	jr nz, loader

	ld a,(0x1003)
	cp #0xC3
	jr nz, badimage
	ld a,(0x1003)
	cp #0xC3
	jr nz, badimage
	ld hl,(0x1006)
	ld de,#0x10AD
	or a
	sbc hl,de
	jr nz, badimage

	ld hl,#gogogo
	call outstr
	ld a,#1			; ZRCC
	jp 0x1000

signon:
	.ascii 'ZRCC Boot Loader'
	.db 13,10,0
gogogo:
	.ascii 'done'
	.db 13,10,0

badimage:
	ld hl,#badbadbad
	call outstr
	halt

badbadbad:
	.ascii 'Invalid'
	.db 13,10,0

;
;	Load sector d from disk into HL and advance HL accordingly
;
load_sector:
	ld a,d
	out (0x13),a		; LBA
	ld a,#1
	out (0x12),a		; 1 sector
	ld a,#0x20
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

outstr:
	ld a,(hl)
	or a
	ret z
	call outchar
	inc hl
	jr outstr
	
;
; Based on the ROM code but slightly tighter
; - use ld a,#0 so 0 and 1 bits are same length
; - don't duplicate excess code in the hi/lo bit paths
; - use conditional calls to keep 0/1 timing identical
;
; FIXME: my math says it's still slightly off timing.
;
outchar:
	push bc
	ld c,a
	ld b,#8
	call lobit
	ld a,c
txbit:
	rrca
	call c, hibit
	call nc, lobit
	djnz txbit
	pop bc
hibit:
	push af
	ld a,#0xff
	out (0xf9),a
	ld a,#7
bitwait:
	dec a
	jp nz,bitwait
	pop af
	ret
lobit:
	push af
	ld a,#0
	out (0xf9),a
	ld a,#7
	dec a
	jp bitwait
