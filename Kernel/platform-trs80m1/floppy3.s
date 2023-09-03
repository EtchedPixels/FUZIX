;
;	TRS80 Model III floppy driver
;
;	Slightly fun because we use double density media and nmi and halt
;	tricks
;
;	Based on the Model 4 driver
;
;	FIXME: better drive spin up wait
;	FIXME: double sided media
;	FIXME: correct step rates (per drive ?)
;	FIXME: precompensation
;		- not on single density
;		- track dependant for double density based on trsdos dir pos
;	FIXME: fix the tiny motor/start command race. I think it's safe
;		but check
;
;
	.globl _fd3_restore
	.globl _fd3_operation
	.globl _fd3_motor_on
	.globl _fd3_motor_off
	.globl fd_nmi_handler
	.globl _fd_map
	.globl _fd_selected
	.globl _fd_tab
	.globl _fd_cmd
	.globl map_kernel, map_proc_always
	.globl nmi_handler

	.module floppy3

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
;	FIXME:
;	fdc_active must never be set when interrupts are enabled or we may
;	take a motor off during an interrupt...
;
;	Nasties:
;	The model 3 executes the following code on an NMI (plus some misc
;	jumps in the process)
;	IN A(E4)	; corrupting A of whatever is running
;	BIT 5,A
;	JP NZ, 4049	; if not reset button user vector
;
;	So we can only enable the NMI in tightly controlled situations
;
fd_nmi_handler:
	ld 	a, (fdc_active)
	or	a
	jp 	z, nmi_handler	; Dead
	xor	a
	out	(FDCINT), a
	push	bc
	ld	bc, #100
	call	nap
	pop	bc
	pop	af
	pop	af		; discard return address
	jp	fdio_nmiout	; and jump

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
	cp	#2
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
	nop				; 44
fdxferdone:
	xor	a			;
	ld	(fdc_active), a		; 
	out	(FDCINT),a		; 
	ei				;
	in	a, (FDCREG)
	and	#0x19			; Error bits + busy
	bit	0, a			; Wait for busy to drop, return in a
	ret	z
	ld	a, (fdcctrl)
	out	(FDCCTRL), a		; turn off halt flag
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
;
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
	ld	b, #0xFE
fdio_outbyte:
	out	(FDCCTRL), a		; stalls
	outi
	jp	nz,fdio_outbyte
fdio_outpatch:
	nop
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

fdxferbad:
	ld	a, #0xff
	ret

;
;	C glue interface.
;
;	Reset to track 0, wait for the command then idle
;
;	fd_reset3(uint8_t *drvptr)
;
_fd3_restore:
	ld	a, (fdcctrl)
	out	(FDCCTRL), a
	ld	a, #1
	out	(FDCSEC), a
	xor	a
	out	(FDCTRK), a
	ld	a, #0x0C	; FIXME: seek rate
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
;	fd_operation3(uint8_t *driveptr)
;
;	The caller must ensure the drive has been selected and the motor is
;	running.
;
_fd3_operation:
	ex	de,hl		; arg into de
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
;	C interface fd_motor_on(uint16_t drivesel)
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
_fd3_motor_on:
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
	and	#0x7F
	out 	(FDCCTRL), a
	out	(FDCCTRL), a	; TRS80 erratum: model 4 gate array apparently needs this
	ld	(fdcctrl), a
	ld	bc, #0x7F00	; Long delay (may need FE or FF for some disks?)
	call	nap
	; FIXME: longer motor spin up delay goes here (0.5 or 1 second)
	
	call	waitdisk
;
;	All is actually good
;
	ld	a,#1
	ld	(motor_running),a
motor_was_on:
	ld	hl, #0
	ret

;
;	C interface fd_motor_off(void)
;
;	Turns off the drive motors, deselects all drives
;
_fd3_motor_off:
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
