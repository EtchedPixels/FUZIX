;
;	We are running at 0x2000 so the final work we do requires some care
;	that we don't blow ourselves up.
;

	.area _BOOT(ABS)
	.org 0x2000

	; We get run with a valid SP but it's lurking in high space so
	; move sp for banking.
start:
	ld (escape),sp
	ld sp,#0x8000
	xor a
	rst 8
	.db 0x89
	ld (drive),a

	ld hl, #filename
	ld b, #0x01
	ld a,(drive)

	rst 8
	.db 0x9A
	
	jr c, failure		; C???

	ld (handle),a

	; Target for 0-3FFF
	ld a,#0x16		; Stuff the first 16K in bank 6
	ld hl,#0xC000
	call load16k

	; Target for 4000-7FFF
	ld a,#0x14		; Second 16K for 4000-7FFF into page 4
	ld hl,#0xC000
	call load16k

	; Target for 8000-BFFF
	ld a,#0x10
	ld hl,#0x8000
	call load16k	; Load 8000-BFFF in situ

	ld a,#0x10
	ld hl,#0xC000
	call load16k	; Load C000-FFFF in situ (CODE1 BANK0)

	ld a,#0x11	
	ld hl,#0xC000
	call load16k	; Load C000-FFFF in situ (CODE2 BANK1)

	ld a,#0x17
	ld hl,#0xC000
	call load16k	; Load C000-FFFF in situ (CODE3 BANK 7)

	ld a,(handle)
	rst 8
	.db 0x9B

	; We can now commit violence

	ld a,#0x0C	; bank 4, alt screen
	ld bc,#0x7ffd
	out (c),a	; Alt screen on to hide the evidence

	; We are blowing away stuff the ROM interrupt routine wants to
	; touch so stop it

	di

	; We must not touch the stack from here until we change sp

	; Move 4000-7FFF into place
	ld hl,#0xC000
	ld de,#0x4000
	ld b,d
	ld c,e
	ldir

	; Next we want bank 6 where we hid the low stuff

	ld bc,#0x7ffd
	ld a,#0x0E
	out (c),a

	ld hl,#strap
	ld de,#0x4000
	ld bc,#0x0100
	ldir

	; Now the delicate bit of the operation - taking over the DivMMC/IDE
	; ROM image
	jp 0x4000		; vector for the final strap code which
				; moves the DivMMC bits into place and
				; runs the image. We have this loaded
				; over a bit of video space

load16k:
	ld bc,#0x7FFD
	out (c),a		; paging for this load

	ld a,(handle)
	ld bc,#0x4000
	rst 8
	.byte 0x9D
	ret nc
failure:
	ld hl,#ohpoo
fail:
	ld a,(hl)
	inc hl
	or a
	jr z, faildone
	rst 0x10
	jr fail
	; throw ourselves under the bus
faildone:
	ld a,(handle)
	or a
	jr z, noclose
	rst 8
	.byte 0x9B
noclose:
	ld sp,(escape)
	ret

ohpoo:
	.ascii 'Unable to load Fuzix image'
	.db 13,0
filename:
	.asciz 'FUZIX.BIN'
handle:
	.db 0
drive:
	.db 0
escape:
	.dw 0

;
;	Code run from 0x4000
;

strap:

	ld a,#0x80
	out (0xE3),a		; ESX EPROM forced mapped low
	ld a,#0xC9		; RET
	ld (0x3D00),a		; autopage range
	call 0x3D00		; force page
	ld a,#0x83		; ESX EPROM and page 3
	out (0xE3),a
	ld hl,#0xC000		; Move the low 8K into page 3
	ld de,#0x2000
	ld b,d
	ld c,e
	ldir
	ld a,#0x40
	out (0xE3),a		; MAPRAM mode
	ld de,#0x2000		; Move the last 8K into place
	ld b,d
	ld c,e
	ldir

	; The eagle has landed

	ld a,#0x10		; bank 0 alt screen
	ld bc,#0x7ffd
	out (c),a		; memory as we want it
	ld hl,#0x2200
	; 0x2200 should start with a signature of ZB then the execute
	; address
	ld a,(hl)
	cp #'Z'
	jr nz, failed
	inc hl
	ld a,(hl)
	cp #'B'
	jr nz, failed
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	jp (hl)
failed:
	; The machine is busted by this point
	di
	halt
