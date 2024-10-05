;
;	Core floppy routines for the Dragon
;
;	Must live in common space as they are called from interrupt
;	contexts and also map user space about
;

	.globl	fd_nmi_handler
	.globl	nmi_handler

	.globl _fd_reset
	.globl _fd_operation
	.globl _fd_motor_on
	.globl _fd_motor_off

	.globl _fd_tab
;
;	MMIO for the floppy controller
;
;	For a Dragon cartridge
;
FDCCTRL	EQU	0xFF48
;
;	0-1: drive select
;	2: motor on
;	3: density
;	4: precomp
;	5: nmi mode
;
FDCREG	EQU	0xFF40
FDCTRK	EQU	0xFF41
FDCSEC	EQU	0xFF42
FDCDATA	EQU	0xFF43

;
;	Structures we use
;
;
;	Per disk structure to hold device state
;
TRKCOPY	EQU	0

;
;	Command issue
;
CMD	EQU	0
TRACK	EQU	1
SECTOR	EQU	2
DIRECT	EQU	3		; 0 = read 2 = write 1 = status
DATA	EQU	4

	.area	.common
;
;	NMI handling for the floppy drive
;
fd_nmi_handler:
	lda	FDCREG
	ldy	nmivector
	sty	10,s		; overwrite the return PC
	ldy	#nmi_handler
	sty	nmivector	; unexpected NMI trap
	rti

;
;	Snooze to give the drive controller time to think
;
disknap:
	ldx	#8750		; assuming 0.9MHz
disknapw:
	leax	-1,x
	bne	disknapw
	rts

;
;	Wait for the drive controller to become ready
;
waitdisk:
	ldx	#0		; wait timer
waitdisk_l:
	leax	-1,x
	beq	forceint	; try forcing an interrupt
	lda	<FDCREG
	bita	#0x01
	bne	waitdisk_l
	rts			; done, idle EQ true
forceint:			; no response, bigger stick
	lda	#0xD0		; reset
	sta	<FDCREG
	nop
	exg	a,a
	exg	a,a
	lda	<FDCREG		; read to reset int status
	; ?? what to do next ??
	lda	#0xff		; force NEQ
	rts

;	Set up the disk. On entry y points to our per drive data and
;	x points to the command block
;
fdsetup:
	lda	TRKCOPY,y
	sta	<FDCTRK		; reset track register
	pshs	x,y
	cmpa	TRACK,x		; target track
	beq	fdiosetup

	lda	TRACK,x
	sta	<FDCDATA	; target
	;
	;	So we can verify
	;
	lda	SECTOR,x
	sta	<FDCSEC
	;
	;	Need to seek the disk
	;
	lda	#0x14
	sta	<FDCREG		; seek
	nop
	exg	a,a
	exg	a,a
	jsr	waitdisk
	bne	setuptimeout
	jsr	disknap
	anda	#0x18		; error bits
	beq	fdiosetup
	; seek failed, not good
setuptimeout:			; NE = bad
	puls	x,y
	ldb	<FDCTRK		; we have no idea where we are
	stb	TRKCOPY,y	; so remember what the drive reported
	rts
;
;	Head in the right place
;
fdiosetup:
	puls	x,y
	lda	TRACK,x
	sta	TRKCOPY,y	; remember we arrived
	ldb	fdcctrl
	andb	#0xEF		; precomp
	cmpa	#22
	blo	noprecomp
	orb	#0x10
noprecomp:
	orb	#0x20		; NMI/halt on
	stb	<FDCCTRL		; precomp configured
	lda	SECTOR,x
	sta	<FDCSEC
	lda	<FDCREG		; clear any pending int
	lda	CMD,x		; command to issue
	ldy	#fdxferdone
	sty	nmivector	; so our NMI handler will clean up
	ldy	#0		; timeout handling
	orcc	#0x50		; irqs off or we'll miss bytes
	ldb	DIRECT,x
	cmpb	#0x01		; read ?
	beq	fdio_in
	cmpb	#0x02
	beq	fdio_out	; write
	sta	<FDCREG		; issue the command
	nop			; give the FDC a moment to think
	exg	a,a
	exg	a,a
;
;	Status registers
;
fdxferdone:
	ldb	fdcctrl
	stb	<FDCCTRL
	lda	<FDCREG
	anda	#0x7C		; Returns with A holding the status bits
	rts
;
;	Relies on B being 2...
;
fdio_out:
	sta	<FDCREG		; issue the command
	ldx	DATA,x		; get the data pointer
	lda	,x+		; otherwise we don't have time
				; to fetch it
wait_drq:
	ldb	<0xFF23		; check for DRQ the fastest way we can
	bmi	drq_go		; go go go...
	leay	-1,y
	bne	wait_drq
	;
	;	Timed out - reset etc to clean up ??
	;
	lda	#0xff		; our error code
	rts

;
;	Once the controller decides it is finished it will flag an NMI
;	and the NMI will switch the PC on the return to fdxferdone.
;
;	Begin the actual copy to disk
;
drq_loop:
	sync
drq_go:
	sta	<FDCDATA
	ldb	<0xFF22		; clear the FIR (PIA1DB)
	lda	,x+
	bra	drq_loop	; exit is via NMI

;
;	Read from the disk
;
fdio_in:
	sta	<FDCREG		; issue the command
	ldx	DATA,x
fdio_dwait:
	ldb	<0xFF23		; wait on the PIA not fd regs.. quicker
	bmi	fdio_go
;
;	Not ready, go round again
;
	leay	-1,y
	bne	fdio_dwait
;
;	FIXME: do error recovery at some point (reset/poll)
;
	ldb	fdcctrl
	stb	<FDCCTRL
	lda	#0xff
	rts

fdio_go:
	lda	<FDCDATA	; get the data first
	ldb	<0xFF22		; clear FIR
	sta	,x+		; store the received byte
fdio_loop:
	sync			; stall for FIR
	ldb	<0xFF22		; clear the FIR (PIA1DB)
	lda	<FDCDATA
	sta	,x+
	bra	fdio_loop	; NMI terminates this


;
;	PIA management
;
piasave:
	pshs	x,y
	ldx	#0xFF01		; PIA0CRA
	ldy	#pia_stash
	lda	,x
	sta	,y+
	anda	#0xFC
	sta	,x++		; move on to 0CRB
	lda	,x
	sta	,y+
	anda	#0xFC
	sta	,x
	leax	0x1e,x		; PIA1CRA
	lda	,x
	sta	,y+
	anda	#0xFC
	sta	,x++		; on to PIA1CRB
	lda	,x
	sta	,y
	ora	#0x37
	sta	,x		; floppy FIR enabled
	puls	x,y,pc

piaload:
	; Must leave B untouched
	pshs	x,y
	ldx	#0xFF01
	ldy	#pia_stash
	lda	,y+
	sta	,x++
	lda	,y+
	sta	,x
	leax	0x1e,x
	lda	,y+
	sta	,x++
	lda	,y+
	sta	,x
	puls	x,y,pc
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
;	fd_reset(uint16_t *drive)
;
_fd_reset:
	pshs	x,y,dp
	lda	#0xFF
	tfr	a,dp
	ldb	fdcctrl
	stb	<FDCCTRL
	lda	#0x01
	sta	<FDCSEC
	lda	#0x00		; seek
	sta	<FDCTRK
	sta	<FDCREG
	nop
	exg	a,a
	exg	a,a
	jsr	waitdisk
	jsr	disknap
	; FIXME: longer delay goes here ???
	cmpa	#0xff
	beq	rstff		; Total fail
	anda	#0x10		; Error bit from the reset
rstff:
	tfr 	a,b
	puls	x,y,dp,pc
;
;	fd_operation(uint16_t *cmd, uint16_t *drive)
;
;	The caller must ensure the drive has been selected and the motor is
;	running.
;
_fd_operation:
	pshs y,cc,dp
	lda	#0xFF
	tfr	a,dp
	orcc	#0x40		; Make sure FIR is off
	jsr	piasave
	ldy	6,s		; Drive struct
	tst	,x+		; User or kernel ?
	beq	fd_op_k
	jsr	map_proc_always
fd_op_k:
	jsr	fdsetup		; Set up for a command
	tfr	a,b		; Status code or 0xFF for total failure
	jsr	map_kernel
	bsr	piaload
	puls	y,cc,dp,pc	; Restore IRQ state etc
;
;	C interface fd_motor_on(uint8 drivesel)
;
;	Selects this drive and turns on the motors
;
_fd_motor_on:
	pshs	y,dp
	lda	#0xFF
	tfr	a,dp

	;
	;	Select drive B, turn on motor if needed
	;
	lda	motor_running	; nothing selected
	beq	notsel
	cmpb	curdrive	; check if we are good
	bne	notsel
;
;	All is actually good
;
motor_was_on:
	ldb	#0
	puls	y,dp,pc
;
;	Select our drive
;
notsel:
	orb	#0x04		; motor on, single density + our drive id
	stb	<FDCCTRL
	stb	fdcctrl
	bita	#0x4
	bne	motor_was_on
	jsr	disknap
	; FIXME: longer motor spin up delay goes here
	jsr	waitdisk
	tfr	a,b		; return in the right place
	puls	y,dp,pc

;
;	C interface fd_motor_off(void)
;
;	Turns off the drive motors, deselects all drives
;
_fd_motor_off:
	pshs	y,dp
	lda	#0xFF
	tfr	a,dp

;
;	Deselect drives and turn off motor
;
	ldb	motor_running
	beq	no_work_motor
	; Should we seek to track 0 ?
	ldb	<FDCCTRL
	andb	#0xF0
	stb	<FDCCTRL
	clr	motor_running
no_work_motor:
	puls y,dp,pc

;
;	We need these mapped during interrupts so must live in common
;
	.area .commondata
nmivector:
	.word	nmi_handler
curdrive:
	.byte	0xff
;
;	BSS but used with user mapping so keep common
;
	.area .commondata
motor_running:
	.byte	0
fdcctrl:
	.byte	0
pia_stash:
	.byte	0
	.byte	0
	.byte	0
	.byte	0
_fd_tab:
	.byte	0xFF,0xFF,0xFF,0xFF
