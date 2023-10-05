.z80
;
;	TRS80 bootblock (256 bytes)
;
;	The model 3 bootblock is loaded at 0x4300 not 0x4200 as the model 1
;	does. It's also double density and sector counts start at 1.
;
;	As we are going to blow away the existing setup and it's damned
;	inconvenient have different load OS addresses by model we relocate
;	ourselves down to 0x4200 and run from there
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
;	FIXME: Does this need to be a double density boot block and reader ?
;	40 track SS/DD is known to work does 40 track SS/SD still work ?
;
;
.area	    BOOT	(ABS)
.org	    0x4200
;
;	The 1791 is I/O mapped but has halt logic to meeting timings.
;
FDCREG	.equ	0xF0
FDCTRK	.equ	0xF1
FDCSEC	.equ	0xF2
FDCDATA	.equ	0xF3

FDCSEL	.equ	0xF4			; and watch errata

start:
	    ld hl,#0x4300
	    ld de,#0x4200
	    ld bc,#0x0100
	    ldir
	    jp go
go:
;	    call 0x04C3			; 64 columns
	    ; Stack is at 407D
   	    ; Loader message
	    ld de, #0x3C00
	    call prints
	    .ascii 'TRS80Load 0.2m3\0'
	    out (0x43),a		; a is 0 on prints return
	    in a,(0x43)
	    or a
	    jr z, bank_ok
	    call printse
	    .ascii 'Supermem not found\0'
bad:	    jr bad

bank_ok:
	    ld hl, #0x4300		; page aligned buffer we read into
lastsec:    ld a, #0			; self modifying to save space
	    inc a
	    ld (lastsec+1), a		; track number... (start on 1)
	    out (FDCDATA),a		; Desired track into data
	    ld a,#0x81
	    out (FDCSEL),a		; MFM drive 1 no halt
	    ld a,#0x1B
	    out (FDCREG),a		; seek, lowest step speed
	    ex (sp),hl
	    ex (sp),hl
	    ex (sp),hl
	    ex (sp),hl
seekstat:
	    in a,(FDCREG)		; Wait for busy to drop
	    rra
	    jr c,  seekstat
	    in a,(FDCREG)		; Did it work ?
	    and #0x18
	    jr z, secmove
	    call printse
	    .ascii 'seek\0'
bad2:	    jr bad2

secmove:    xor a
	    ld (nextsec+1), a
nextsec:
	    ld a, #0		; self modifying sector count
	    inc a
	    ld (nextsec+1), a
	    cp #19
	    jr z, lastsec
	    call floppy_read
	    push hl
	    ld l,h
	    ld h,#0x3D
	    inc (hl)
	    pop hl
	    jr nextsec
;
;	Assume the boot ROM left us on track 0 and motor on
;
floppy_read:
	    out (FDCSEC), a		; sector please
	    ld a, #0x81			; drive 1 select, dd, side 0
	    out (FDCSEL), a		; keep motor on
	    ld a, #0x80			; READ
	    out (FDCREG), a
	    ex (sp),hl			; 19: We need to wait 58 clocks
	    ex (sp),hl			; 38: not a lot more, no less
	    ld de,#0xC102		; 48: D is used for the select
					; drive 1, double density, side 0.
					; wait
	    ld bc,#FDCDATA		; 58: For the ini
flopin:	    in a, (FDCREG)
	    and e
	    jr z, flopin
	    ini				; First data byte
	    di
	    ld a,d			; drive/density etc
flopinl:
	    out	(FDCSEL),a		; stall for byte
	    ini				; fetch
	    jp	nz, flopinl
	    ld a,h
	    or l
	    jr nz, flopstat		; passed FFFF ?
	    ld h,#0x80			; go back to 8000 and change bank
	    in a,(0x43)
	    or a
	    jr nz, booted		; already on second bank so boot
	    ld a,#1			; switch to bank 2
            out (0x43),a
flopstat:
	    in a, (FDCREG)
	    and #0x19
	    rra				; test bit 0
	    jr nc, flopdone
	    ld a,d
	    out (FDCSEL),a
	    jr flopstat
flopdone:
	    or a			; safe even though we rotated right
	    ret z
	    call printse
	    .ascii 'Read\0'
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
	    .ascii 'Booting..\0'
	   xor a
	   jr 0x4300
