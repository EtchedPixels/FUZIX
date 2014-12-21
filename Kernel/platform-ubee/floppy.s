;
;	Core floppy routines for the TRS80 1791 FDC
;	Based on the 6809 code
;
;	FIXME: double sided media
;	FIXME: correct step rates (per drive ?)
;	FIXME: precompensation
;	FIXME: density, size etc
;
	.globl _fd_reset
	.globl _fd_operation
	.globl _fd_motor_on
	.globl fd_nmi_handler

FDCREG	.equ	0x40
FDCTRK	.equ	0x41
FDCSEC	.equ	0x42
FDCDATA	.equ	0x43
FDCCTRL	.equ	0x48

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

	.area	_COMMONMEM
;
;	Simple routine for pauses
;
nap:	dec	bc
	ld	a, b
	or	c
	jr	nz, nap
	ret
;
;	NMI logic for DreamDisc
;
fd_nmi_handler:
	push	af
	push	bc
	ld 	a, (fdc_active)
	or	a
	jr 	z, boring_nmi
	pop	bc
	pop	af
	pop	af		; discard return address
	jp	fdio_nmiout	; and jump

;
;	FIXME: check for motor off here
;
boring_nmi:
	pop	bc
	pop	af
	retn
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
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	djnz	waitdisk_l
	dec	c
	jr	nz, waitdisk_l
	ld	a, #0xD0	; reset
	out	(FDCREG), a
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
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
fdsetup:
;
;	Are we on side 0 or side 1 ?
;
	ld	a, SECTOR(ix)
	cp 	#11
	jr	c, side0
;
;	Set up for side 1 access
;
	sub	#10
	ld	SECTOR(ix), a
	ld	a, (fdcctrl)
	set	2, a		; correct for standard FDC not DreamDisc
	jr	side1
side0:
	ld	a, (fdcctrl)
	res	2, a
side1:
;
;	FIXME: do we need to precompensation ?
;
	ld	(fdcctrl), a
	out	(FDCCTRL), a

	ld	a, (de)
	out	(FDCTRK), a
	cp	TRACK(ix)
;	jr	z, fdiosetup

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
	ld	a, #0x19	; seek at 6ms steps. Slightly conservative
				; to allow for older drives
	out	(FDCREG), a
	ld	b, #100
seekwt:	djnz	seekwt
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

	ld	a, CMD(ix)

	ld	de, #0		; timeout handling
	
	out	(FDCREG), a	; issue the command
	ld	b, #0
rwiowt:	djnz	rwiowt
	ld	a, DIRECT(ix)
	dec	a
	ld	a, (fdcctrl)
	ld	d, a			; we need this in a register
					; to meet timing
	ld	a, #1
	ld	(fdc_active), a		; NMI pop and jump
	jr	z, fdio_in
	jr	nc, fdio_out
;
;	Status registers
;
fdxferdone:
	ei
fdxferdone2:
	xor	a
	ld	(fdc_active), a
	in	a, (FDCREG)
	and	#0x19		; Error bits + busy
	bit	0, a		; Wait for busy to drop, return in a
	ret	z
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	jr	fdxferdone2
;
;	Read from the disk - HL points to the target buffer
;
fdio_in:
	ld	e, #0x16		; bits to check
	ld	bc, #FDCDATA		; 512 bytes/sector, c is our port
fdio_inl:
	in	a, (FDCREG)
	and	e
	jr	z, fdio_inl
	ini
	di
	ld	a, d
fdio_inbyte:
	out	(FDCCTRL), a		; stalls
	ini
	jr	nz, fdio_inbyte
fdio_inbyte2:
	out	(FDCCTRL), a		; stalls
	ini
	jr	nz, fdio_inbyte2
	jr	fdxferdone

;
;	Read from the disk - HL points to the target buffer
;
fdio_out:
	set	6,d			; halt mode bit
	ld	c, #FDCDATA		; C is our port
	ld	e, #0x76
fdio_outl:
	in	a, (FDCREG)		; Wait for DRQ (or error)
	and	e
	jr	z, fdio_outl
	outi				; Stuff byte into FDC while we think
	di
	in	a, (FDCREG)		; No longer busy ??
	rra
	jr	nc, fdxferbad		; Bugger... 
	ld	b, (hl)			; Next byte
	inc	hl
fdio_waitlock:
	ld	a, d
	out	(FDCCTRL), a		; wait states on
	in	a, (FDCREG)
	and	e
	jr	z, fdio_waitlock
	out	(c), b
	ld	a, d
fdio_outbyte:
	out	(FDCCTRL), a		; stalls
	outi
	jr	fdio_outbyte
fdio_nmiout:
;
;	Now tidy up
;
	jr	fdxferdone

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
	pop	de
	pop	hl
	push	hl
	push	de
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	ld	a, #1
	out	(FDCSEC), a
	xor	a
	out	(FDCTRK), a
	ld	a, #0x0D	; use 6ms stepping
	out	(FDCREG), a	; restore
	ld	a, #0xFF
	ld	(hl), a		; Zap track pointer
	ld	b, #0
_fdr_wait:
	djnz	_fdr_wait
	
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
	pop	bc		; return address
	pop	hl		; command
	pop	de		; drive track ptr
	push	de
	push	hl
	push	bc
	push	ix
	push	hl
	pop	ix
	ld	l, DATA(ix)
	ld	h, DATA+1(ix)
	call	fdsetup		; Set up for a command
	ld	l, a
	ld	h, #0
	pop	ix
	ret
;
;	C interface fd_motor_on(uint16_t drivesel)
;
;	Selects this drive and turns on the motors. Also pass in the
;	choice of density. For the standard uBee disk that is
;
;	bits 0-1:	select drive
;	bit 2:		side (must rewrite each drive change)
;			(handled elsewhere)
;	bit 3:		density (set = DD)
;
;
_fd_motor_on:
	pop	de
	pop	hl
	push	hl
	push	de
	;
	;	Select drive B
	;
	cp	l
	jr	z,  was_selected
;
;	Select our drive
;
notsel:
	ld	a, l
	out 	(FDCCTRL), a
	out	(FDCCTRL), a	; TRS80 erratum apparently needs this
	ld	(fdcctrl), a
	ld	bc, #0x7F00	; Long delay (may need FE or FF for some disks)
	call	nap
	; FIXME: longer motor spin up delay goes here (0.5 or 1 second)
	
	call	waitdisk
;
;	All is actually good
;
was_selected:
	ld	hl, #0
	ret

curdrive:
	.db	0xff
fdcctrl:
	.db	0
fdc_active:
	.db	0
