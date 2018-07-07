.z80
;
;	TRS80 bootblock (256 bytes)
;
;
;	We load from 0x4300-0xFFFF into common and bank 0, then switch
;	to loading from 0x8000-0xFFFF in bank1. That takes 320 sectors
;	at 10 sectors per track so 32 tracks of those available (which may
;	only be 35). Track 0 holds the boot block.
;
;	We could optimize this once we have the layout better planned so we
;	load less empty space. On the other hand we've only got 256 bytes
;	of space to play in.
;
;	We have zero free bytes, so to add anything you need to squash
;	it a bit.
;
.area	    BOOT	(ABS)
.org	    0x4200

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
	    call 0x04C3			; 64 columns
	    ; Stack is at 407D
   	    ; Loader message
	    ld de, #0x3C00
	    call prints
	    .ascii 'TRS80LOAD 0.3M1\0'
	    out (0x43),a		; a is 0 on prints return
	    in a,(0x43)
	    or a
	    jr z, bank_ok
	    ld hl,(3)
	    ld bc,#0x011f
	    ld a,c
	    ld (patch+3),a		; patch the port
	    ld a,#0x08
	    ld (patch+1),a		; patch the value
	    ld a,b
	    ld (patch2+1),a		; pass the banker type
	    out (c),b
	    ld de,(3)
	    dec b
	    out (c),b
	    or a
	    sbc hl,de
	    jr nz, bank_ok
	    call printse
	    .ascii 'BANKED MEMORY NOT FOUND\0'
bad:	    jr bad
second_block:
	    .byte  0			; if we need to save use bank_ok+1 !
bank_ok:
	    ld bc, #0x4300		; page aligned buffer we read into
	    ld de, #FDCDATA		; data port
	    ld hl, #FDCREG		; command port

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
	    .ascii 'seek\0'
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
	    push hl
	    ld h,#0x3D
	    ld l,b
	    inc (hl)
	    pop hl
	    jr nextsec
;
;	Assume the boot ROM left us on track 0 and motor on
;
floppy_read:
	    ld (FDCSEC), a		; sector please
	    ld a, #0x01			; drive 0 select
	    ld (LATCHD0), a		; keep motor on
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
	    inc b			; correct bc for next call
	    jr nz, flopstat		; passed FFFF ?
	    ld b,#0x80			; go back to 8000 and change bank
	    ld a,(second_block)
	    or a
	    jr nz, booted		; already on second bank so boot
	    ld a,#1			; switch to bank 2
	    ld (second_block),a
patch:	    ld a,#1			; patch for non supermem
            out (0x43),a
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
patch2:	   ld a,#0
