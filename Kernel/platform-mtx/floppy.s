;
;	Core floppy routines for the Memotech SDX (WD17xx)
;	Based on the 6809 code
;
;	FIXME: better drive spin up wait
;	FIXME: double sided media
;	FIXME: correct step rates (per drive ?)
;	FIXME: precompensation
;
;
	.module	floppy

	.globl _fd_reset
	.globl _fd_operation
	.globl _fd_motor_on
	.globl _fd_motor_off
	.globl _fd_cmd
	.globl _fd_data

	.globl map_proc_always
	.globl map_kernel

FDCREG	.equ	0x10
FDCTRK	.equ	0x11
FDCSEC	.equ	0x12
FDCDATA	.equ	0x13
FDCCTRL	.equ	0x14
;
;	interrupt register reports 0x80 for interrut, 0x40 for drq
;	(0x20 is the unrelated reset button)
;

;
;	Structures we use
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
	ld	a, (de)
	out	(FDCTRK), a
	cp	TRACK(ix)
	jr	z, fdiosetup
	;
	;	So we can verify
	;
	bit	4, SECTOR(ix)	; side ?
	ld	a, (fdcctrl)
	jr	z, seekside1
	set	1, a
	jr 	seekside
seekside1:
	res	1, a
seekside:
	out	(FDCCTRL), a
	ld	a, SECTOR(ix)
	and 	#15
	inc a
	out	(FDCSEC), a
	ld	a, TRACK(ix)
	out	(FDCDATA), a	; target
	;
	;	Need to seek the disk
	;
	ld	a, #0x14	; seek
	out	(FDCREG), a
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	call	waitdisk
	jr	nz, setuptimeout
	and	#0x18		; error bits
	jr	z, seekgood
	; seek failed, not good
setuptimeout:			; NE = bad
	ld	a, #0xff	; we have no idea where we are, force a seek
	ld	(de), a		; zap track info
	ret
;
;	Seek worked ok
;
seekgood:
	in	a, (FDCREG)
	ld	a, TRACK(ix)
	ld	(de), a		; save track
;
;	Head in the right place
;
fdiosetup:
;	cmp	#22		; FIXME
;	jr	nc, noprecomp
;	ld	a, (fdcctrl)
;	or	#0x10		; Precomp on
;	jr	precomp1
;noprecomp:
	ld	a, (fdcctrl)
;precomp1:
	bit	4, SECTOR(ix)	; check if we need side 1
	jr	nz, fdio_s1
	res	1, a
	jr	fdio_setsec
fdio_s1:set	1, a
fdio_setsec:
	out	(FDCCTRL), a
	ld	a, SECTOR(ix)
	and 	#15
	inc	a		; 1 based
	out	(FDCSEC), a
	in	a, (FDCREG)	; Clear any pending status

	ld	a, CMD(ix)

	ld	de, #0		; timeout handling
	
	out	(FDCREG), a	; issue the command
	ex	(sp),hl	; give the FDC a moment to think
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ld	a, DIRECT(ix)
	dec	a
	ld	a, (fdcctrl)
	ld	d, a			; we need this in a register
					; to meet timing
	set	6,d			; halt mode bit
	jr	z, fdio_in
	jr	nc, fdio_out
;
;	Status registers
;
fdxferdone:
	ei
fdxferdone2:
	in	a, (FDCREG)
	and	#0x19		; Error bits + busy
	bit	0, a		; Wait for busy to drop, return in a
	ret	z
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	jr	fdxferdone2
;
;	Write to the disk - HL points to the target buffer
;
fdio_in:
	ld	e, #0x16		; bits to check
	ld	bc, #FDCDATA		; 256 bytes/sector, c is our port
fdio_inl:
	in	a, (FDCREG)		; wait for data ready
	and	e
	jr	z, fdio_inl
	di
	ini				; grab and go
fdio_inl2:
	in	a, (FDCREG)
	and	e
	jr	z, fdio_inl2
	ini
	jr	nz, fdio_inl2
	jr	fdxferdone

;
;	Read from the disk - HL points to the target buffer
;
fdio_out:
	ld	bc, #FDCDATA		; 256 bytes/sector, c is our port
	ld	e, #0x76
fdio_outl:
	in	a, (FDCREG)		; Wait for DRQ (or error)
	and	e
	jr	z, fdio_outl
	outi				; Stuff byte into FDC while we think
	di
	in	a, (FDCREG)		; No longer busy ??
	bit	0,a
	jr	nz, fdio_outbyte
fdxferbad:				; Bugger... 
	ld	a, #0xff
	ret
fdio_outbyte:
	in	a, (FDCREG)
	and	e
	jr	z, fdio_outbyte
	outi
	jr	nz, fdio_outbyte
	jr	fdxferdone


;
;	C glue interface.
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
	out	(FDCREG), a	; restore
	dec	a
	ld	(hl), a		; Zap track pointer
	ex	(sp),hl		; give the FDC a moment to think
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	
	call	waitdisk
	cp	#0xff
	ret	z
	and	#0x10		; Error bit from the reset
	ret	nz
	ld	(hl), a		; Track 0 correctly hit
	ret
;
;	fd_operation(uint16_t *cmd, uint16_t *drive)
;
;	The caller must ensure the drive has been selected and the motor is
;	running.
;
_fd_operation:
	pop	bc		; return address
	pop	de		; drive track ptr
	push	de
	push	bc

	push	ix		; save IX (caller save in SDCC)

	ld	hl, (_fd_data)	 ; data ptr
	ld 	a, (_fd_cmd)	 ; command type
	ld	ix, #_fd_cmd + 1 ; command
	or	a
	push	af
	call	nz, map_proc_always
	call	fdsetup		; Set up for a command
	ld	l, a
	ld	h, #0
	pop	af
	call	nz, map_kernel
	pop	ix
	ret
;
;	C interface fd_motor_on(uint16_t drivesel)
;
;	Selects this drive and turns on the motors. Also pass in the
;	choice of density
;
;	bit 0: drive select
;	bit 1: side select
;	bit 2: motor on
;	bit 3: motor ready
;	bit 4:
;
;
_fd_motor_on:
	pop	de
	pop	hl
	push	hl
	push	de
	;
	;	Select drive B, turn on motor if needed
	;
	ld	a,(motor_running)	; nothing selected
	or	a
	jr	z, notsel

	cp	l
	jr	z,  motor_was_on
;
;	Select our drive
;
notsel:
	ld	h, a		; save state as it was
	or	l
	out 	(FDCCTRL), a
	ld	(fdcctrl), a
	bit	2, h		; FIXME - motor bit
	jr	nz, motor_was_on
	ld	bc, #8000
motorwait:
	in	a, (FDCCTRL)
	bit	3, a
	jr	nz, motor_good
	dec	bc
	ld	a, b
	or	c
	jr	nz, motorwait
;
;	Timed out
;
	ld	hl, #-1
	ret
;
;	All is actually good ? 
;
;	If we find the motor is not good try spinning up
;
motor_was_on:
	in	a, (FDCCTRL)
	bit	3,a
	jr 	z, notsel
	
motor_good:
	ld	hl, #0
	ret

;
;	C interface fd_motor_off(void)
;
;	Turns off the drive motors, deselects all drives
;
_fd_motor_off:
	ld	a, (motor_running)
	or	a
	ret	z
	; Should we seek to track 0 ?
	in	a, (FDCCTRL)
	res	2,a
	out	(FDCCTRL), a
	xor	a
	ld	(motor_running), a
	ret

curdrive:
	.db	0xff
motor_running:
	.db	0
fdcctrl:
	.db	0
_fd_cmd:
	.ds	5
_fd_data:
	.dw	0
