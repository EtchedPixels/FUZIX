;
;	Core floppy routines for the TRS80 1791 FDC
;	Based on the 6809 code
;
;	FIXME: better drive spin up wait
;	FIXME: double sided media
;	FIXME: correct step rates (per drive ?)
;	FIXME: precompensation
;		- not on single density
;		- track dependant for double density based on trsdos dir pos
;
;
	.globl _fd_restore
	.globl _fd_operation
	.globl _fd_motor_on
	.globl _fd_motor_off
	.globl fd_nmi_handler
	.globl _fd_map
	.globl _fd_selected
	.globl _fd_tab
	.globl _fd_cmd
	.globl map_kernel, map_proc_always

	.module floppy

FDCREG	.equ	0xF0
FDCTRK	.equ	0xF1
FDCSEC	.equ	0xF2
FDCDATA	.equ	0xF3
FDCCTRL	.equ	0xF4
FDCINT	.equ	0xE4
;
;	interrupt register reports 0x80 for interrupt, 0x40 for drq
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
SIZE	.equ	6		; For now 1 = 256 2 = 512
STEP	.equ	7		; Step rate
COMP	.equ	8		; Write compensation
HEAD	.equ	9

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
;	The motor off logic is driven from hardware
;
;	fdc_active must never be set when interrupts are enabled or we may
;	take a motor off during an interrupt...
;
fd_nmi_handler:
	push	af
	push	bc
	ld 	a, (fdc_active)
	or	a
	jr 	z, boring_nmi
	xor	a
	out	(FDCINT), a
	ld	bc, #100
	call	nap
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
	ld	a,#0xff
	ret
;
;	Set up and perform a disk operation
;
;	IX points to the command block
;	HL points to the buffer
;	DE points to the track reg copy
;
fdsetup:
	ld	a, (fdcctrl)
	and	#0xE0		; mask off side
	ld	b,a
	xor	a
	cp	HEAD(ix)
	jr	z, headlow
	set	4,b		; side 1
headlow:
	ld	a,b
	ld	(fdcctrl),a
	out	(FDCCTRL),a
	ld	a, (de)
	out	(FDCTRK), a
	cp	TRACK(ix)
	jr	z, fdiosetup
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
	ld	a, #0x18	; seek
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
	ld	bc,#0x0518		; patch in a skip over the out and
	ld	de,#0x0718		; JR + 7
	ld	a, SIZE(ix)
	cp	#1
	jr	z, patchfor256
	ld	bc,#(FDCCTRL * 256 + 0xD3)	; out (FDCCTRL),a
	ld	de,#0x0000			; nop, nop
patchfor256:
	ld	(fdio_inbyte2),bc		; second loop
	ld	(fdio_outpatch),de

	ld	a, TRACK(ix)
	ld	(de), a		; save track
	cp	COMP(ix)
	ld	a, (fdcctrl)
	jr	nc, noprecomp
	or	#0x10		; Precomp on
noprecomp:
	out	(FDCCTRL), a
	ld	a, SECTOR(ix)
	out	(FDCSEC), a
	in	a, (FDCREG)	; Clear any pending status

	ld	a, (fdcctrl)		; has halt bit set as passed
	ld	d, a			; we need this in a register
					; to meet timing
	ld	a, #1			; 
	ld	(fdc_active), a		; NMI pop and jump

	ld	bc, #FDCDATA		; 256 or 512 bytes/sector, c is our port

	ld	a, CMD(ix)
	out	(FDCREG), a		; issue the command
	;
	;	Now count the clocks. We need 58 between here and the actual
	;	data poll
	;
	ld	a, DIRECT(ix)		; 19
	dec	a			; 23
	jr	z, fdio_in		; 30/35	 12/7
	jp	nc, fdio_out		; 40	 10
;
;	And we can afford to be later to non data commands
;
;	Status registers
;
fdxferdone:
	xor	a
	ld	(fdc_active), a
	out	(FDCINT),a		; 
	ei
	in	a, (FDCREG)
	and	#0x19		; Error bits + busy
	bit	0, a		; Wait for busy to drop, return in a
	ret	z
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	jr	fdxferdone
;
;	Read from the disk - HL points to the target buffer
;	When we hit this section we are 35 clocks into the sequence
;	and have to hit the first in at about clock 58
;
fdio_in:
	ld	a, #0x16		; 42 bits to check
	ld	e, a			; 46 into e
	set	6,d			; 54 ensure halt is on
	nop				; 58
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
	jp	nz, fdio_inbyte
fdio_inbyte2:				; this is patched for I/O size
	out	(FDCCTRL), a		; stalls
	ini
	jp	nz, fdio_inbyte2
	jr	fdxferdone

;
;	Write to the disk - HL points to the target buffer
;
;	We arrive here 40 clocks after the command, and we want to hit
;	status at about 58 clocks

fdio_out:
	ld	a, #0x76		; 47
	ld	e,a			; 51
	set	6,d			; 59
	ld	b,#0x0F
fdio_p:	djnz	fdio_p
fdio_outl:
	in	a, (FDCREG)		; Wait for DRQ (or error)
	and	e
	jr	z, fdio_outl
	outi				; Stuff byte into FDC while we think
	di
	in	a, (FDCREG)		; No longer busy ??
	rra
	ret	nc			; Bugger... 
	ld	a, #0xC0		; Turn on magic floppy NMI interface
	out	(FDCINT), a
	ld	b, #50			; Spin for it
spin1:	djnz	spin1
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
	ld	b,#0xFE
fdio_outbyte:
	out	(FDCCTRL), a		; stalls
	outi
	jp	nz,fdio_outbyte
fdio_outpatch:
	nop	; patched with jr + 7
	nop
fdio_outbyte2:
	out	(FDCCTRL), a		; stalls
	outi
	jp	nz,fdio_outbyte2
fdio_nmiout:
;
;	Now tidy up
;
	jr	fdxferdone
;
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
;	fd_restore(uint8_t *drvptr) __fastcall
;
_fd_restore:
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	ld	a, #1
	out	(FDCSEC), a
	xor	a
	out	(FDCTRK), a
	ld	a, #0x0C
	out	(FDCREG), a	; restore
	ld	a, #0xFF
	ld	(hl), a		; Zap track pointer
	ld	b, #0
_fdr_wait:
	djnz	_fdr_wait
	
	call	waitdisk
	ex	de,hl
	ld	l,a
	cp	#0xff
	ret	z
	and	#0x99		; Error bit from the reset
	ret	nz
	ld	(de), a		; Track 0 correctly hit (so 0)
	ld	l,a
	ret
;
;	fd_operation(uint16_t *drive) __fastcall
;
;	The caller must ensure the drive has been selected and the motor is
;	running.
;
_fd_operation:
	ex	de,hl
	ld	a, (_fd_map)
	or	a
	call	nz, map_proc_always
	push	ix
	ld	ix, #_fd_cmd
	ld	l, DATA(ix)
	ld	h, DATA+1(ix)
	call	fdsetup		; Set up for a command
	ld	l, a
	ld	h, #0
	pop	ix
	ld	a, (_fd_map)
	or	a
	ret	z
	jp	map_kernel
;
;	C interface fd_motor_on(uint16_t drivesel) __fastcall
;
;	Selects this drive and turns on the motors. Also pass in the
;	choice of density
;
;	bits 0-3:	select that drive
;	bit 4:		side (must rewrite each drive change)
;	bit 5:		precompensation (not set here but in the I/O ops)
;	bit 6:		synchronize I/O by stalling the CPU (don't set this)
;	bit 7:		set for double density (MFM)
;
;
_fd_motor_on:
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
motor_was_on:
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
	and	#0xF0		; clear drive bits
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
fdc_active:
	.db	0
_fd_map:
	.db	0
_fd_selected:
	.db	0xFF
_fd_tab:
	.db	0xFF, 0xFF, 0xFF, 0xFF
_fd_cmd:
	.ds	10
