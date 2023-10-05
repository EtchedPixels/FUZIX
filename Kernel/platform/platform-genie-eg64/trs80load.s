.z80
;
;	TRS80 bootblock (256 bytes)
;
;	We load from 0x0100-0xFFFF avoiding the I/O and screen area
;
;	We could optimize this once we have the layout better planned so we
;	load less empty space. On the other hand we've only got 256 bytes
;	of space to play in.
;
.area	    BOOT	(ABS)
.org	    0x0000		; loaded at 4200 and ldir's itself

;
;	The 1791 is memory mapped
;
FDCREG	.equ	0x37EC
FDCTRK	.equ	0x37ED
FDCSEC	.equ	0x37EE
FDCDATA	.equ	0x37EF
;
;	Drive select is also more complicated than other models
;
LATCHD0	.equ	0x37E1		; also drive select
LATCHD1	.equ	0x37E3
LATCHD2	.equ	0x37E5
LATCHD3	.equ	0x37E7

start:
	    ;
	    ;	Copy ourself down to 0x0000
	    ;
	    di
	    call 0x04C3			; 64 columns

	    ; Relocate to the bottom of memory
	    ; Set up memory mapping
	    ld a,#0xC0			; RAM low I/O mapped
	    out (0xC0),a		; need to skip I/O (3800-3FFF)

	    ld hl,#0x4200
	    ld e,l
	    ld d,l
	    ld bc,#0x0100
	    ldir
	    ld sp,#0x0100

	    ld a,(0x01)
	    cp #0xAF
	    jp nz, go
	    ld a,#'E'
	    ld (0x3C40),a
stop:	    jr stop


go:
   	    ; Loader message
	    ld de, #0x3C00
	    call prints
	    .ascii 'VGLOAD 0.3EG64\0'

	    ld de, #FDCDATA		; data port
	    ld hl, #FDCREG		; command port
	    ; Loading base address
	    ld bc, #0x0100		; page aligned buffer we read into

lastsec:    ld a, #0			; self modifying to save space
	    inc a
	    ld (lastsec+1), a		; track number... (start on 1)
	    ld (de),a			; Desired track into data
	    ld (hl),#0x1B		; seek, lowest step speed
	    push bc
	    ld b, #0			; 0x00 256 delays
cmd1:	    djnz cmd1
seekstat:
	    pop bc
	    bit 0, (hl)
	    jr nz, seekstat
	    ld a,(hl)
	    and #0x18
	    jr z, secmove
	    call printse
	    .ascii 'SEEK\0'
bad2:	    jr bad2
secmove:    xor a
	    dec a	
	    ld (nextsec+1), a
nextsec:
	    ld a, #255		; self modifying sector count
	    inc a
	    ld (nextsec+1), a
	    cp #10
	    jr z, lastsec
	    call floppy_read
	    jr nextsec
;
;	Assume the boot ROM left us on track 0 and motor on
;
floppy_read:
	    ld (FDCSEC), a		; sector please
	    ld a, #0x01			; drive 0 select
	    ld (LATCHD0), a		; keep motor on
	    ld a, #0x37
	    cp b			; I/O area
	    jr nz, notio
	    ld b,#0x40			; Skip to 0x4000
notio:
	    ld a, #0x8C			; READ
	    ld (hl), a
	    push bc
	    ld b, a			; 8C is an acceptable delay!
l1:	    djnz l1
	    pop bc
flopin:	    bit 1,(hl)
	    jr z, flopin
	    ld a,(de)
	    ld (bc),a
	    inc c			; page aligned
	    jr nz,flopin
	    ; Load up unil 
	    inc b			; correct bc for next call
	    jr z, booted		; passed FFFF ?
flopstat:
	    ld a, (hl)
	    and #0x19
	    rra				; test bit 0
	    jr nz, flopstat
	    or a			; safe even though we rotated right
	    ret z
	    call printse
	    .ascii 'READ\0'
fail:	    jr fail


printse:
	   ld de, #0x3C40
prints:
	   pop hl
	   ld a,#0xC0			;map display
	   out (0xC0),a
printsl:
	   ld a, (hl)
	   inc hl
	   or a
	   jr z, out
	   ld (de), a
	   inc de
	   jr printsl
out:	   jp (hl)

booted:	   call printse
	    .ascii 'BOOTING..\0'
	   jr 0x100
