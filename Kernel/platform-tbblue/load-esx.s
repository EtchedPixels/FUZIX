;
;	TBBlue bootstrap via ESXDOS API
;
;	We are loaded at 0x2000. We load the kernel into banks 16-23 and
;	then map most of it in and jump to 0x0100 in the loaded image
;
;	As we use higher pages we know that we can return to BASIC and
;	stuff like the RAMdisc will remain intact.
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
	
	jr c, failure

	ld (handle),a

	;
	;	We load the image into high pages so that if we barf
	;	everything is good even the ramdisc.
	;
	ld a,#16		; Stuff the first 16K in bank 16/17
	ld hl,#0xC000
	call load16k

	ld a,#18		; Second bank into 18/19
	call load16k

	ld a,#20		; Third bank
	call load16k

	ld a,#22
	call load16k		; Fourth bank

	ld a,(handle)
	rst 8
	.db 0x9B		; Close

	ld hl,#strap
	ld de,#0xFF00
	ld bc,#0x0100
	ldir

	; Now the delicate bit of the operation - taking over the DivMMC/IDE
	; ROM image

	ld hl,#0xFF00
	rst 0x20		; Terminate esxdos command and return to
				; our bootstrap code we just moved

load16k:
	ld bc,#0x243B
	ld e,#0x56		; 0xC000 mapping register
	out (c),e
	inc b			; to data port
	out (c),a		; Page requested
	dec b			; back to control port
	inc e			; next register
	inc a			; next page
	out (c),e		; 0xE000 mapping register
	inc b			; back to data port
	out (c),a		; page

	ld a,(handle)		; Load 16K into that page
	ld bc,#0x4000
	ld hl,#0xC000		; We mapped it at 48K up
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
;	Code run from 0xFF00
;
strap:
	; We are moving interrupt vectors and stuff
	di
	; Page in 16/17/18/19/20/21/22
	; not 23 as that will go over us. Instead the kernel entry will
	; fix that one up
	ld a,#16
	ld bc,#0x243B
	ld de,#0x5007
mmuset:
	out (c),d		; register
	inc b
	out (c),a		; data
	dec b
	inc a			; next page
	inc d			; next register
	dec e			; are we there yet ?
	jr nz, mmuset

	jp 0x0100	
