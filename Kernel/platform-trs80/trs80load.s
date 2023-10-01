.z80
;
;	TRS80 bootblock (256 bytes)
;
.area	    BOOT	(ABS)
.org	    0x000
start:
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
;
;	Assume the boot ROM left us on track 0 and motor on
;
floppy_read:
	    ld bc, #0x81F4		; select drive
	    out (c), b
	    out (0xF2), a		; sector please
	    ld a, #0x80			; READ
	    out (0xF0), a
	    ld b, #100
l1:	    djnz l1
	    ld de, #0x8116
	    ld bc, #0xF3
flopin:	    in a, (0xF0)
	    and e
	    jr z, flopin
	    ini
	    ld a, d
flopind:
	    out (0xF4), a
	    ini
	    jr nz, flopind
flopstat:
	    in a, (0xF0)
	    and #0x19
	    bit 0, a
	    jr nz, flopstat
	    or a
	    ret z
	    ld de, #0xF850
	    call prints
	    .ascii 'Read\0'
fail:	    jr fail
;
go:   
	    ld sp, #0xEE00		; temp stackpointer

            ; load the 6845 parameters
	    ld hl, #_ctc6845		; reverse order
	    ld bc, #0x0F88
ctcloop:    out (c), b			; select register
	    ld a, (hl)
	    out (0x89), a		; output data
	    dec hl
	    djnz ctcloop

   	    ; clear screen
	    ld hl, #0xF800
	    ld de, #0xF801
	    ld bc, #0x07FF
	    ld (hl), #' '
	    ldir
	    ld de, #0xF800
	    call prints
	    .ascii 'TRS80Load 0.2\0'

	    ld hl, #0x0100
;


lastsec:    ld a, (tracknum)
	    inc a
	    ld (tracknum), a
	    cp #40
	    jr z, booted
	    out (0xF3), a
	    ld a, #0x18			; seek
	    out (0xF0), a
	    ld b, #100
cmd1:	    djnz cmd1
cmdwait:    in a, (0xF0)
	    bit 0, a
	    jr z, seekstat
	    bit 7, a
	    jr z, cmdwait
seekstat:
	    in a, (0xF0)
	    and #0x18
	    bit 0, a
	    jr nz, seekstat
	    or a
	    jr z, secmove
	    ld de, #0xF850
	    call prints
	    .ascii 'seek\0'
bad:	    jr bad
secmove:    xor a
	    ld (secnum), a
nextsec:
	    ld a, (secnum)
	    inc a
	    ld (secnum), a
	    cp #19		; was 18
	    jr z, lastsec
	    push hl
	    call floppy_read
	    pop hl
	    inc h
	    jr nextsec

tracknum:   .db 27			; tracks 28-39 (55296 bytes)

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
secnum:	    .db 0		; recycle last crc value as secnum so we fit 256 bytes
_ctc6845:   .db 0


prints:
	   pop hl
printsl:
	   ld a, (hl)
	   inc hl
	   or a
	   jr z, out
	   ld (de), a
	   inc de
	   jr printsl
out:	   jp (hl)

booted:	    ld de, #0xF850
	    call prints
	    .ascii 'Booting..\0'

; OS starts on the following byte
