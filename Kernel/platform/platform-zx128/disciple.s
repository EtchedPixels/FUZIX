;
;	Supporting routines for the DISCiple floppy controller
;
;	This a WD177x controller coupled to an extra magic switching ROM
;	chip with its own RAM. From our perspective the other bits don't
;	matter just the hardware
;
;	The timings for the WD1770 on the spectrum are such that
;	we can't check all the bits each loop. We must also be in
;	an uncontended bank (even numbered) eg _CODE
;
;	Ports
;	0x1B - status/cmd
;	0x5B - track
;	0x9B - sector
;	0xDB - data
;
;	Also uses
;	0xFB - printer data
;	0xBB - read to page in DISCiple ROM/RAM, write to page out
;		*don't do this with IRQ's on for FUZIX*
;	0x7B - read puts the disciple ROM in the low 8K, write puts the
;	       RAM there (the other of the pair lives at 8K)
;
;	0x1F - control (w/o)
;		bit 0: drive 0/1 (1 = drive 0)
;		bit 1: side 0/1 (1 = side 1)
;		bit 2: SD/DD (1 = SD?)
;		bit 3-5: interface paging control
;		bit 6: printer strobe
;		bit 7: network
;
;	Commands are like other WD177x with the usual variation
;	0x78 - step out (for 0 keep stepping out until give up or zero set)
;	0x58 - step in
;	0xA0 - write + 0x02 for spin up etc, + 0x08? for precomp
;	0xD0 - reset (then wait about 1mS)
;
;
;	This code ought to work on the Plus-D as well with a few changes
;
;	The +D uses ports
;	0xE3	status/cmd
;	0xEB	track
;	0xF3	sector
;	0xFB	data
;	0xEF	control
;	bits 0-1: drive select
;	bit 7: side select
;

	.globl _fd_reset
	.globl _fd_operation
	.globl _fd_selected
	.globl _fd_tab
	.globl _fd_cmd

FDCREG	.equ	0x1B
FDCTRK	.equ	0x5B
FDCSEC	.equ	0x9B
FDCDATA	.equ	0xDB

FDCCTRL	.equ	0x1F


		.area _CODE
;
;	The timings on this are ugly and we don't get to do a sanity
;	length check as would be nice. We may need to double buffer user
;	copies as our common is contended memory.
;
;	FIXME: save original HL - subtract and compare with sector size
;
;	Entry: HL = input buffer (sector size)
;	Exit: Z = ok, NZ A = error or 0 for short
;	Uses: AF, BC, HL
;
;
;	Timeout useful ?
;
cmd1772:
	in a, (0x1B)		; read status
	bit 0, a
	jr nz, cmd1772
	ld a,b
	out (0x1B),a		; write the CMD register
	ld b, #32
cmd1772w:
	djnz cmd1772w		; fixme - reduce for DD (test and use 30/15)
	ret

;
;	interrupt register reports 0x80 for interrut, 0x40 for drq
;	(0x20 is the unrelated reset button)
;

;
;	Structures we use
;
;
;	Per disk structure to hold device state
;
TRKCOPY	.equ	0

;
;	Command issue
;
CMD	.equ	0
TRACK	.equ	1
SECTOR	.equ	2
DIRECT	.equ	3		; 0 = read 2 = write 1 = status
DATA	.equ	4

;
;	Wait for the drive controller to become ready
;	Preserve HL, DE
;
waitdisk:
	ld	bc, #0
waitdisk_l:
	in	a, (FDCREG)
	bit	0, a
	ret	z
	;
	;	Keep poking fdcctrl to avoid a hardware motor timeout
	;
	djnz	waitdisk_l
	dec	c
	jr	nz, waitdisk_l
	ld	c, #0xD0	; reset
	call	cmd1772
	in	a, (FDCREG)		; read to reset int status
	bit	0, a
	ret
;
;	Set up and perform a disk operation
;
;	IX points to the command block
;	HL points to the buffer
;	DE points to the track reg copy
;
;	The caller is responsible for setting up the drive and side bits
;	and the precomp fieeld in write commands (tracks > 64 for 80 or
;	> 32 for 40 track drives)
;
fdsetup:
	ld	a, (de)
	out	(FDCTRK), a
	cp	TRACK(ix)
;	jr	z, fdiosetup		; FIXME

	;
	;	So we can verify
	;
	ld	a, TRACK(ix)
	out	(FDCDATA), a
	ld	a, SECTOR(ix)
	out	(FDCSEC), a
	;
	;	Need to seek the disk
	;
	ld	b, #0x18	; seek	 (18 or 10 ?)
	call	cmd1772
	call	waitdisk
	jr	nz, setuptimeout
	and	#0x18		; error bits
	jr	z, fdiosetup
	; seek failed, not good
setuptimeout:			; NE = bad
	ld	a, #0xff	; we have no idea where we are, force a seek
	ld	(de), a		; zap track info
	ret
;
;	Head in the right place
;
fdiosetup:
	ld	a, TRACK(ix)
	ld	(de), a		; save track
	ld	a, SECTOR(ix)
	out	(FDCSEC), a
	in	a, (FDCREG)	; Clear any pending status

	ld	c, CMD(ix)

	ld	de, #0		; timeout handling
	
	call	cmd1772

	ld	a, DIRECT(ix)
	dec	a
	jr	z, fdio_in
	jr	nc, fdio_out
;
;	Status registers
;
fdxferdone:
	ei
fdxferdone2:
	in	a, (FDCREG)
	; FIXME: mask varies by type - WP on write etc
	and	#0x19		; Error bits + busy
	bit	0, a		; Wait for busy to drop, return in a
	ret	z
	jr	fdxferdone2
;
;	Read from the disk - HL points to the target buffer
;
fdio_in:
	ld bc, #0xDB		; 256 count, port DB (data of 1770)
	jr sector_idrq
sector_byte:
	ini			; 16
	nop			; 4  the ROM does - not clear why
sector_idrq:
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_byte	; 7 / 12 if taken
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_byte	; 7 / 12 if taken
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_byte	; 7 / 12 if taken
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_byte	; 7 / 12 if taken
	
	;
	; We should have had bits by now
	;	

	bit 0,a			; 8
	jr nz, sector_idrq	; 7 / 12 if taken

	;
	;	Gone non busy
	;
	and	#0x1C
	jr	fdxferdone

;
;	Read from the disk - HL points to the target buffer
;
fdio_out:
	ld bc, #0xDB		; 256 count, port DB (data of 1770)
	jr sector_odrq
sector_obyte:
	outi			; 16
	nop			; 4  the ROM does - not clear why
sector_odrq:
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_obyte	; 7 / 12 if taken
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_obyte	; 7 / 12 if taken
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_obyte	; 7 / 12 if taken
	in a,(0x1B)		; 11 like all spectrum hw ports splattered
				;    all over
	bit 1,a			; 8
	jr nz, sector_obyte	; 7 / 12 if taken
	
	;
	; We should have done the bits by now
	;	

	bit 0,a			; 8
	jr nz, sector_idrq	; 7 / 12 if taken

	;
	;	Gone non busy
	;
	and #0x5C		; error bits and WP bit (to us an error)
	jr nz, fdxferdone
	xor a			; 256 bytes ??
	cp b
	jr z, fdxferdone
fdxferbad:
	ld	a, #0xff
	ret

;
;	C glue interface.
;
;	Because of the brain dead memory paging we dump the bits into
;	kernel space always. The thought of taking an NMI while in the
;	user memory and bank flipping to recover is just too odious !
;

;
;	Reset to track 0, wait for the command then idle
;
;	fd_reset(uint8_t *drvptr)
;
_fd_reset:
	pop	bc
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	ld	a, #1
	out	(FDCSEC), a
	xor	a
	out	(FDCTRK), a
	ld	c, #0x0C
	call	cmd1772
	ld	a, #0xFF
	ld	(hl), a		; Zap track pointer
	call	waitdisk
	cp	#0xff
	ret	z
	and	#0x99		; Error bit from the reset
	ret	nz
	ld	(hl), a		; Track 0 correctly hit (so 0)
	ret
;
;	fd_operation(uint16_t *cmd, uint16_t *drive)
;
;	The caller must ensure the drive has been selected and the motor is
;	running.
;
_fd_operation:
	pop	hl		; return address
	pop	bc		; padding
	pop	de		; drive track ptr
	push	de
	push	bc
	push	hl
	push	ix
	ld	ix, #_fd_cmd
	ld	l, DATA(ix)
	ld	h, DATA+1(ix)
	call	fdsetup		; Set up for a command
	ld	l, a
	ld	h, #0
	pop	ix
	ret

	.area _COMMONDATA
curdrive:
	.db	0xff
fdcctrl:
	.db	0
fdc_active:
	.db	0
_fd_selected:
	.db	0xFF
_fd_tab:
	.db	0xFF, 0xFF
_fd_cmd:
	.ds	6
