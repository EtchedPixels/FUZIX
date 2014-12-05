;
;	Core floppy routines for the Dragon
;

	.globl	fd_nmi_handler
	.globl	nmi_handler

	.globl _fd_reset
	.globl _fd_operation
	.globl _fd_motor_on
	.globl _fd_motor_off
;
;	MMIO for the floppy controller
;
;
;	These four are a normal WD2797 under DragonDOS
;
FDCREG	EQU	0xFF40
FDCTRK	EQU	0xFF41
FDCSEC	EQU	0xFF42
FDCDATA	EQU	0xFF43
;
;	This is the control logic
;
FDCCTRL	EQU	0xFF48		; drive select and motors
; 	bit5: NMI enable, 4: Precomp ?, 3: Density, 2: Motor, 
;	bits 0-1 are drive nymber 0-3


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
DRIVESEL EQU	3
DIRECT	EQU	4		; 0 = read 2 = write 1 = status
DATA	EQU	5

	.area	.text
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
	lda	FDCREG
	bita	#0x01
	bne	waitdisk_l
	rts			; done, idle EQ true
forceint:			; no response, bigger stick
	lda	#0xD0		; reset
	sta	FDCREG
	nop
	exg	a,a
	exg	a,a
	lda	FDCREG		; read to reset int status
	; ?? what to do next ??
	lda	#0xff		; force NEQ
	rts

;
;	Set up the disk. On entry y points to our per drive data and
;	x points to the command block
;
fdsetup:
	lda	TRKCOPY,y
	sta	FDCTRK		; reset track register
	pshs	x,y
	cmpa	TRACK,x		; target track
	beq	fdiosetup
	;
	;	Need to seek the disk
	;
	lda	#0x14
	sta	FDCREG		; seek
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
	lda	FDCTRK		; we have no idea where we are
	sta	TRKCOPY,y	; so remember what the drive reported
	rts
;
;	Head in the right place
;
fdiosetup:
	puls	x,y
	lda	TRACK,x
	sta	TRKCOPY,y	; remember we arrived
	ldb	FDCCTRL
	andb	#0xEF
	cmpa	#22
	blo	noprecomp
	orb	#0x10
noprecomp:
	stb	FDCCTRL		; precomp configured
	lda	SECTOR,x
	sta	FDCSEC
	lda	FDCREG		; clear any pending int
	lda	CMD,x		; command to issue
	ldy	#fdxferdone
	sty	nmivector	; so our NMI handler will clean up
	ldy	#0		; timeout handling
	orcc	#0x10		; irqs off or we'll miss bytes
	sta	FDCREG		; issue the command
	nop			; give the FDC a moment to think
	exg	a,a
	exg	a,a
	ldb	DIRECT,x
	cmpb	#0x01		; read ?
	beq	fdio_in
	cmpb	#0x02
	beq	wait_drq	; write
;
;	Status registers
;
fdxferdone:
	andcc	#0xef
	lda	FDCREG
	anda	#0x7C		; Returns with A holding the status bits
	rts
;
;	Relies on B being 2...
;
wait_drq:
	bitb	FDCREG
	bne	drq_on
	leay	-1,y
	bne	wait_drq
	;
	;	Timed out - reset etc to clean up ??
	;
	andcc	#0xef
	lda	#0xff		; our error code
	rts

;
;	Once the controller decides it is finished it will flag an NMI
;	and the NMI will switch the PC on the return to fdxferdone.
;
;	Begin the actual copy
;
drq_on:
	ldy	DATA,x
drq_loop:
	ldb	,y+
	stb	FDCDATA		; hardware will stall this for us
	sta	FDCREG
	bra	drq_loop

;
;	Write to the disk
;
fdio_in:
	ldy	DATA,x
fdio_loop:
	ldb	FDCDATA		; Sync is done by hardware
	stb	,y+
	bra	fdio_loop	; exit is via NMI to fdxferdone


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
	pshs	x,y
	lda	#0x00		; seek
	sta	FDCREG
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
	tfr a,b
	puls	x, y, pc
;
;	fd_operation(uint16_t *cmd, uint16_t *drive)
;
;	The caller must ensure the drive has been selected and the motor is
;	running.
;
_fd_operation:
	pshs y
	ldy 4,s		; Drive struct
	jsr fdsetup	; Set up for a command
	puls y
	tfr a,b		; Status code or 0xFF for total failure
	rts
;
;	C interface fd_motor_on(uint8 drive)
;
;	Selects this drive and turns on the motors
;
_fd_motor_on:
	pshs y
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
	puls	y,pc
;
;	Select our drive
;
notsel:
	orb	#0xA8		; NMI, motor on, density + our drive id
	lda	FDCCTRL
	stb	FDCCTRL
	bita	#0x08
	bne	motor_was_on
	jsr	disknap
	; FIXME: longer motor spin up delay goes here
	jsr	waitdisk
	tfr a,b		; return in the right place
	puls y,pc

;
;	C interface fd_motor_off(void)
;
;	Turns off the drive motors, deselects all drives
;
_fd_motor_off:
	pshs y
;
;	Deselect drives and turn off motor
;
	ldb	motor_running
	beq	no_work_motor
	; Should we seek to track 0 ?
	ldb	FDCCTRL
	andb	#0xF0
	stb	FDCCTRL
	clr	motor_running
no_work_motor:
	puls y,pc

	.area .data

nmivector:
	.word	nmi_handler
curdrive:
	.byte	0xff

	.area .bss
motor_running:
	.byte	0
