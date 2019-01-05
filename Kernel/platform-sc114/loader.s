;
;	This will need our own SCM module as well because the CP/M loader
;	moves CP/M and trashes SCM before running it.
;
;	Currently this and the glue driver code is well under 2 disk blocks.
;	Adding partitions should still fit 2 blocks nicely.
;
	.area _CODE

;
;	Magic for the loader
;
	.byte 'Z'
	.byte 80

;
;	Loader core
;
	.include '../filo/filo.s'
;
; ****************************************************************************
;
;	System specific code begins here
;
; ****************************************************************************

DATA		.equ 	0x10
ERROR		.equ	0x11
FEATURES	.equ	0x11
COUNT		.equ	0x12
LBA_0		.equ	0x13
LBA_1		.equ	0x14
LBA_2		.equ	0x15
LBA_3		.equ	0x16
STATUS		.equ	0x17
CMD		.equ	0x17

ERR		.equ	0
DRQ		.equ	3
READY		.equ	6
BUSY		.equ	7

ATA_READ	.equ	0x10

drive		.equ	0xE0		; Drive 0 for now


SC114		.equ	8
SC108		.equ	0		; until assigned

;
;	bread_disk
;
;	Load a disk block (512 bytes)
;
;	HL = buffer to read into
;	DE = block relative to partbase
;
;	return
;	NZ = fail, Z = ok
;	HL = next read address
;	BC, DE, AF destroyed
;	IX, IY preserved
;
;	For the SCM systems we have to hit the disk directly as the SCM
;	doesn't expose disk functionality at this point. For CP/M or ROMWBW
;	we could go via the monitor.
;
bread_disk:
	push hl
	ld hl,(partbase_l)
	add hl,de
	ld de,(partbase_h)
	jr nc, nocarry_lba
	inc de
nocarry_lba:
	call ide_ready
	; Loading block DEHL
	ld a,d
	and #0x0F
	ld d,a
	ld a,(drive)
	or d
	out (LBA_3),a
	ld a,e
	out (LBA_2),a
	ld a,h
	out (LBA_1),a
	ld a,l
	out (LBA_0),a
	call ide_ready
	ret nz
	pop hl
	ld a,#1
	out (COUNT),a
	ld a,#ATA_READ
	out (CMD),a
	nop
	call ide_wait_drq
	ret nz
	ld bc,#DATA
	inir
	inir
	xor a
	ret
;
;	Timeout might be a good idea
;
ide_ready:
	in a,(STATUS)
	bit ERR,a
	ret nz
	bit READY,a
	jr z, ide_ready
	xor a
	ret
ide_wait_drq:
	in a,(STATUS)
	bit ERR,a
	ret nz
	bit DRQ,a
	jr z,ide_wait_drq
	xor a
	ret

;
;	Drive Set
;
;	A = drive 0-15
;
;	For now just drive A
;
drive_set:
	or a
	ret z
	ld hl,#bad_drive
	jr con_write

bad_drive:
	.asciz 'Invalid drive.\n'
;
;
;	Console I/O
;
;	Read the name buffer into a buffer
;
;	Destroys AF, BC, DE
;
;	Returns length of buffer in A and sets Z/NZ accordingly
;	Buffer pointer is returned in HL
;
;	We implement the console via the SCM monitor helpers
;
con_readbuf:
	ld c,#0x05
	call mcall
	or a
	ex de,hl
	ret
;
;	Check for input
;
;	Returns NZ if data
;	Destroys AF,BC,DE,HL
;
con_check:
	ld c,#0x03
	jp mcall
;
;
;	Write the string at HL
;
;	Destroys AF,HL
;
con_write:
	push bc
	push de
con_writel:
	ld a,(hl)
	cp #0x0A
	jr z, con_writenl
	or a
	jr z, con_writedone
	ld c,#0x02
con_wop:
	push hl
	call mcall
	pop hl
	inc hl
	jr con_writel
con_writenl:
	ld c,#0x07
	jr con_wop
con_writedone:
	pop de
	pop bc
	ret
;
;	Pre boot. Run before anything else kicks off
;
preboot:
	ld c,#0x08
	call mcall
	ld a,h
	cp #SC108
	ret z
	cp #SC114
	ret z
	ld hl,#unsupported
	call con_write
	ld c,#0x00
	jp mcall

unsupported:
	.asciz 'Unsupported platform.\n'

;
;	System.
;	Input HL = command string
;
;	Return: none
;
;	Destroys: AF/BC/DE/HL
;
;	Do anything with a command beginning *. In our case we feed it to
;	the monitor
;
system:
	inc hl		; Skip the *
	ex de,hl
	ld c,#0x22	; Monitor call
	;
	;	Call monitor
	;
mcall:
	ex af,af'
	ld a,#1
	out (0x38),a
	ex af,af'
	rst 0x30
	ex af,af'
	xor a
	out (0x38),a
	ex af,af'
	ret
;
;	Delay approximately 10ms
;
;	Destroys AF/BC/DE/HL
;
;	SCM has a helper for this.
;
delay10ms:
	ld c,#0x0a
	ld de,#0x10
	jr mcall
