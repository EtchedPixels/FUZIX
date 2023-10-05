;
;	Core floppy routines for the TRS80 1791 FDC
;	Based on the 6809 code
;
;	FIXME: better drive spin up wait
;	FIXME: tandy doubler
;	FIXME: correct step rates (per drive ?)
;	FIXME: precompensation ??
;	FIXME: 512 byte sector support
;
	.globl _fd_reset
	.globl _fd_operation
	.globl _fd_motor_on
	.globl _fd_motor_off
	.globl _fd_map
	.globl _fd_selected
	.globl _fd_tab
	.globl _fd_cmd
	.globl map_kernel_restore, map_proc_always
	.globl go_fast, go_slow

	.module floppy
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

;
;	And the select is arranged as
;	bit 0-3 select drives, bit 3 also selects side (so you can't have
;	a disk 3 with any double sided drive)
;

;
;	The final bit of weirdness is magic writes control the doubler. This
;	actually *changes* the chip currently connected to those addresses
;	so things like track must be written in the right order!
;
; Percom (to cmd port)
P_SET_FM 	.equ	0xFE
P_SET_MFM 	.equ	0xFF
; Tandy (to track register)
T_SET_FM	.equ	0xA0
T_SET_MFM	.equ	0x80
T_UNSET_PRECOMP	.equ	0xC0
T_SET_PRECOMP	.equ	0xE0

;
;	It's our responsibility to wait 1 second for motor on and to allow
;	80ms for head load. (and we can call 0060 for 14.65 * BC us delay)
;

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

	.area	_COMMONMEM
;
;	Set up and perform a disk operation
;
;	IX points to the command block
;	BC points to the buffer
;	DE points to the track reg copy
;
;	Drive must already be selected and density set up
;
fdsetup:
	ld	hl,#FDCTRK
	ld	a, (de)
	ld	(hl), a			; Load track register
	cp	TRACK(ix)
	jr	z, fdiosetup		; Is it the one we wanted
	;
	;	So we can verify
	;
	inc	hl			; now FDCTRK
	ld	a, TRACK(ix)
	ld	(hl), a
	inc	hl			; now FDCSEC
	ld	a, SECTOR(ix)
	ld	(hl), a
	;
	;	Need to seek the disk
	;
	ld	hl,#FDCREG
	ld	a, #0x18	; seek	FIXME: need to set step rate
	ld	(hl), a
	call	waitcmd
	and	#0x18		; error bits
	jr	z, fdiosetup
	; seek failed, not good
setuptimeout:			; NE = bad
	ld	a, #0xff	; we have no idea where we are, force a seek
	ld	(de), a		; zap track info
	ld	l,#0xFF		; report failure
	ret
;
;	Try and kick the controller back into sanity
;
bad_cmd:
	ld	a,(hl)		; clear status
	ld	(hl),#0xD0	; force interrupt
	pop	af
	ld	l,#255
	ret
	
waitcmd:
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl		; 87 clocks + the call (17) = 104
waitcmdl:
	bit	0,(hl)
	jr	z, waitfail
	rlca
	ld	a,(last_drive)	; Keep the motor spinning
	ld	(LATCHD0),a
	jr	c, waitcmdl
	ld	a,(hl)		; Status
	ret
waitfail:
	ld	a,#255
	ret
;
;	Head in the right place
;
fdiosetup:
	ld	a, TRACK(ix)
	ld	(de), a		; save track

	; FIXME: select which controller first etc

	ld	de, #FDCSEC	; sector register
	ld	a, SECTOR(ix)
	ld	(FDCSEC), a

	inc	de		; now points at FDCDATA

	ld	hl, #FDCREG	; status/cmd
	ld	a,(hl)		; clear status
	
	ld	a, DIRECT(ix)
	dec	a
	ld	a, CMD(ix)	; 0 - none, 1 - in, 2 = out
	ld	(hl),a
	push	af		; 11
	ex	(sp),hl		; 30		Delay 55us (98 clocks)
	ex	(sp),hl		; 49
	ex	(sp),hl		; 68
	ex	(sp),hl		; 87
	add	a,#0		; 94
	di			; 98
	bit 	0,(hl)		; controller busy ?
	jr	z, bad_cmd
	pop 	af
	jr	z, fdio_in
	jr	nc, fdio_out
	jr	fd_xferdone
;
;	Read from the disk - HL points to the target buffer. This is very
;	tight timing on a 2MHz Z80 with a doubler.
;
;	We point HL at control/status, and DE at the data port. We can't do
;	this with IX offsets as we don't have enough clocks for it.
;
fdio_in:
fdio_do_in:
	ld	a,#0x83			; Wait for the controller to go
	and	(hl)			; ready
	jp	po, fdio_do_in
fdio_xfer_in:
	ld	a,(de)			; 7     data from the port ASAP
	ld	(bc),a			; 7     to memory
	inc	bc			; 6	aligned buffer would be 4
	;
	;	Old trick. What we are effectively doing is synchronizing
	;	the controller and CPU knowing the worst case misalignment
	;
	;	We simply don't have time for this to be a loop
	;
	;	Our best case is 44 clocks per loop so 25us / loop. We have
	;	31us when running double density so will alternate between
	;	44 clock loops and 63 clock (35us) loops.
	;
	bit	1,(hl)			; 12 clocks
	jr	nz, fdio_xfer_in		; 7 clocks if not taken
	bit	1,(hl)
	jr	nz, fdio_xfer_in
	bit	1,(hl)
	jr	nz, fdio_xfer_in
	bit	1,(hl)			; we may have a CPU speed upgrade
	jr	nz, fdio_xfer_in		; fitted...
	bit	1,(hl)
	jr	nz, fdio_xfer_in
	bit	1,(hl)
	jr	nz, fdio_xfer_in
	;
	;	If another byte hasn't turned up in 114 clocks then either
	;	it failed or it finished. Until that happens we can't afford
	;	to check anything else
	;
fd_xferdone:
	ld	l,(hl)			; read the status
	ld	(hl),#0xD0		; force interrupt
	ei
	ret				; pass C code the status byte

;
;	Very similar to the above but going the other way
;
fdio_out:
	ld	(hl),a			; issue command
fdio_do_out:
	ld	a,#0x83			; Wait for the controller to go
	and	(hl)			; ready
	jp	po, fdio_out
fdio_xfer_out:
	ld	a,(bc)			; 7     data from the user
	ld	(de),a			; 7     to the port
	inc	bc			; 6	aligned buffer would be 4
	;
	;	Old trick. What we are effectively doing is synchronizing
	;	the controller and CPU knowing the worst case misalignment
	;
	;	We simply don't have time for this to be a loop
	;
	;	Our best case is 44 clocks per loop so 25us / loop. We have
	;	31us when running double density so will alternate between
	;	44 clock loops and 63 clock (35us) loops.
	;
	bit	1,(hl)			; 12 clocks
	jr	nz, fdio_xfer_out		; 7 clocks if not taken
	bit	1,(hl)
	jr	nz, fdio_xfer_out
	bit	1,(hl)
	jr	nz, fdio_xfer_out
	bit	1,(hl)			; we may have a CPU speed upgrade
	jr	nz, fdio_xfer_out		; fitted...
	bit	1,(hl)
	jr	nz, fdio_xfer_out
	bit	1,(hl)
	jr	nz, fdio_xfer_out
;
;	Now tidy up
;
	jr	fd_xferdone


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
	call	go_slow
	ld	a, #1
	ld	(FDCSEC), a
	xor	a
	ld	(FDCTRK), a
	ld	a, #0x0C
	ld	(FDCREG), a	; restore
	ld	a, #0xFF
	ld	(hl), a		; Zap track pointer
	call	waitcmd
	call	go_fast
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
	pop	de		; drive track ptr
	push	de
	push	bc
	push	ix
	ld	a, (_fd_map)
	or	a
	push	af
	call	nz, map_proc_always
	call	go_slow
	ld	ix, #_fd_cmd
	ld	c, DATA(ix)
	ld	b, DATA+1(ix)
	call	fdsetup		; Set up for a command
	ld	h, #0
	call	go_fast
	pop	af
	pop	ix
	ret	z
	jp	map_kernel_restore
;
;	Delay loops
;
wait45:				; wait 45ms for head load. HL points to 
	ld	bc,#2662	; latch - D is latch value, preserve HL/DE
wait45l:
	dec	bc		; 6	30 cycles per loop (16.9us)
	ld	a,b		; 4
	or	c		; 4
	jr	nz, wait45l	; 12/7
	ret

;
;	Wait about 500ms while poking the motor to keep it spinning.
;
waitdisk:
	ld	b,#11
waitdiskl:
	push	bc
	call	wait45l
	ld	(hl),d		; tickle the motor
	pop	bc
	djnz	waitdiskl
	ret
;
;
;	C interface fd_motor_on(uint16_t drivesel)
;
;	Selects this drive and turns on the motors. Also pass in the
;	choice of density
;
;	bits 0-3:	select that drive (yes side and drive 3 clash)
;	bit 3:		side (must rewrite each drive change)
;	bit 7:		set for double density (MFM)
;
;
_fd_motor_on:
	pop	de
	pop	bc
	push	bc
	push	de

	;
	;	Is the motor running ?
	;
	ld	a,(LATCHD0)
	ld	e, a		; save the latch status (motor on bit)
	rlca
	ld	a,(last_drive)
	jr	nc, must_config	; if the motor is off always do set up
	;
	;	Are we changing our selection ?
	;
	cp	c
	jr	nz, must_config
	;
	;	Motor running, same configuration. Poke the selection
	;	so the motor stays running.
	;
	ld	(LATCHD0),a
	ret

must_config:
	call	go_slow
	ld	a,c		; Save the new configuration
	ld	(last_drive),a

	ld	hl,#FDCREG

	and	#0x7F		; We borrowed bit 7 for our own use
	ld	(LATCHD0), a	; Selects the actual disk we want
	ld	d,a		; Save latch value
	rl	c		; Bit 7 into C
	ld	a,#0xFE		; Figure out the density
	adc	a,#0		; FE or FF according to density
	di
	ld	(hl),a		; if a doubler is present this switches FDC
	; Do we need a delay here (eg if there is no doubler present)
	; and do we need to avoid the FE/FF scribbles on doubler-less hw
	; as we are writing twice to the FDC a few clocks apart otherwise
	ld	(hl),#0xD0	; Hit it over the head with a hammer
	ei

	ld	hl,#LATCHD0	; used by waitdisk too

	bit	7,e		; was the motor running
	jr	z, motor_running

	call	waitdisk	; wait 500ms or so for spin up
	call	go_fast
motor_running:
	ld	(hl),d
	call	wait45		; Wait 45ms for the head to load
	ld	(hl),d		; Reset timer
	ret

;
;	C interface fd_motor_off(void)
;
;	Turns off the drive motors, deselects all drives
;
;	Not sure we need this.
;
_fd_motor_off:
	xor	a
	ld	(LATCHD0),a
	ret

last_drive:
	.db	0xff
_fd_map:
	.db	0
_fd_selected:
	.db	0xFF
_fd_tab:
	.db	0xFF, 0xFF, 0xFF, 0xFF
_fd_cmd:
	.ds	7
