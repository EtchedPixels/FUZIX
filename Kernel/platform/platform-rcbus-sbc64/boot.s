;
;	BOOT and restart image
;
;	We are entered at power on with bank = 3
;

resume_tag .equ 0x0084

	.area _BOOT(ABS)

	.org 0x0000
start:
	jp init

	.org 0x0080
init:  
	di
	ld sp,#0x1000		; safe spot

	ld hl,#signon
	call outstr

	; See if we are loading an image or resuming from memory
	or a
	ld hl,(resume_tag)
	ld de,#0xC0DE
	sbc hl,de
	jr nz, disk_load
	ld a,(0x1003)
	cp #0xC3
	jr nz, disk_load
	ld hl,#resuming
	call outstr
	jp 0x1003

signon:
	.ascii 'SBC64 Boot Loader'
	.db 13,10,0
resuming:
	.ascii 'Resuming from memory'
	.db 13,10,0


disk_load:
	ld hl,#loading
	call outstr
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
	xor a
	jp 0x1000

loading:
	.asciz 'Loading image from disk...'
gogogo:
	.ascii 'done'
	.db 13,10,0

badimage:
	ld hl,#badbadbad
	call outstr
	halt

badbadbad:
	.ascii 'Invalid image or CF problem'
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
