
	.area _BOOT(ABS)

	.org 0xB000

installer:
	ld a,#0x03
	out (0x1f),a
	ld hl,#begin
	call outstr
	ld hl,#data
	ld de,#0x0000
	ld bc,#0x1000
	ldir
	ld hl,#done
	call outstr
	rst 0

begin:
	.asciz 'Configuring booter...'
done:
	.ascii 'Done'
	.db 13,10,0

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

data:
	.include "loader.inc"
