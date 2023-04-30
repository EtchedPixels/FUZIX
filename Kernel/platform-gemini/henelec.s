	.module henelec

;
;	The original NASCOM floppy controller. Single density hooked up over
;	the PIO ports so it didn't need an expansion chassis of any kind
;	Data is wired to the FDC and a drive select latch. Data ends up
;	inverted.
;
;	The Henelec was a user designed device sold as a Henry's Electric
;	kit and then by Gemini as the GM805.
;
;	If you've met the PPIDE then it's the same essential model
;
;	PIO A is wired to the data bus as D0-D7 but inverting
;	PIO B is wired to the control lines as follows
;	bit 7: DRQ
;	bit 6: IRQ
;	bit 5: reset
;	bit 4: !wr for the drive select latch
;	bit 3: !wr for the fdc
;	bit 2: !rd for the fdc
;	bits 1/0: A1/A0 for the FDC
;
;	PIO B is set up once into bit mode with 7-6 input 5-0 output and
;	PIO A is normally input but for writes goes to output
;
;	The drive select latch selects between drives based upon bits
;	bit 3: drive 2
;	bit 2: drive 1
;	bit 1: drive 0
;	bit 0: side select
;
;	The system is usually plugged into a 35 track DSSD drive
;	35 x 18 x 128 5.25". It does apparently support 8"
;
;
;	TODO: format
;	Format causes another problem - you can't 'seek' to an unformatted
;	track so we'll need _hfd_stepin as well. Then you can
;	_hfd_restore ioctl
;	generate track data
;	_hfd_format ioctl
;	read to check
;	_hfd_stepin
;	rinse/repeat
;
;	FIXME: align methods with gm8x9.s so we can just jump table them
;
;	FIXME: port addresses are different between a GM813 and a Nascom
;
	;
	;	I/O methods
	;
	.globl _hfd_select
	.globl _hfd_io
	.globl _hfd_reset
	.globl _hfd_restore

	;
	;	Imports
	;
	.globl _hfd_track
	.globl _hfd_buffer
	.globl _hfd_seccmd
	.globl _io_page

	.include "kernel.def"
	.include "../kernel-z80.def"

	.area _CODE

init_pio:
	; Program the PIO
	; Make our data port input
	ld a,#0xFF
	out (pioa_ctrl),a
	out (pioa_ctrl),a
	; And our control port bit controlled
	out (piob_data),a
	out (piob_ctrl),a
	ld a,#0xC0		; DRQ and IRQ are input lines
	out (piob_ctrl),a
	ret

;
;	Select drive B
;
_hfd_select:
	ld a,(_hfd_drive)
henelec_drvsel:
	ld a,#0x3F
	out (piob_data),a	; ensure all write/read enables are off
	ld a,b
	ld (pioa_data),a
	ld a,#0x1f
	out (piob_data),a	; write enable for the latch, latches 0
	ld a,#0x3f
	out (piob_data),a	; write enable back off
	ld a,#0xff
	out (pioa_ctrl),a	; back to being an input
	out (pioa_ctrl),a
	call sleep100ms
	ret

;
;	We might be able to squash more of this out of commonmem
;
	.area _COMMONMEM
;
;	Return value from port c in b 
;
henelec_in:
	ld a,c
	out (piob_data),a		; addres bits
;
;	We always I think (maybe not data xfer - check)
;	put these back in out methods, so could skip here ?
;
	ld a,#0xFF
	out (pioa_ctrl),a
	inc a
	out (pioa_ctrl),a		; input mode
	ld a,c
	res 2,c				; pulse RE
	out (piob_data),a
	in a,(pioa_data)
	cpl
	or a				; does cpl set these FIXME
	ld b,a
	ld a,c
	out (piob_data),a		; pulse over
	ret

;
;	Send command B to register C
;
henelec_out:
	ld a,c
	out (piob_data),a		; address bits
	ld a,#0xFF		; data to output
	out (pioa_ctrl),a
	out (pioa_ctrl),a
	ld a,b			; gets inverted
	cpl
	out (pioa_data),a		; data on data lines
	ld a,c
	res 3,c			; pulse WE
	out (piob_data),a
	ld a,c
	out (piob_data),a
	ld a,#0xFF		; back to input
	out (pioa_ctrl),a
	inc a
	out (pioa_ctrl),a
	ret

henelec_reset:
	;
	; This magic sequence is what we see DDOS do 
	;
	; Data to output
	ld a,#0xFF
	out (pioa_ctrl),a
	inc a
	out (pioa_ctrl),a
	ld b,a			; b = 0
	call henelec_drvsel
	ld bc,#SEC_REG
	out (c),b
	call wait_a_second
	;	
	; Waggle the reset line
	;
	ld a,#0x2F
	out (piob_data),a
	;
	; 1771 delay
	;
	ld b,#20
l1:	djnz l1
	;
	; Wait for 1771 to go ready
	;
	ld a,#0x3F
;
;	Wait for motor spin up
;
wait_ready:
	push bc
	ld c,#STAREG
	call henelec_in
	jp p,motor_running
	; Poke sector register (seems to write crap to it)
motor_up:
	ld c,#SECREG
	call henelec_out
	; And sleep
	call wait_a_second
	; And see if we are ready
motor_running:
	ld c,#STAREG
	call henelec_in
	jp m,motor_up
	jp c,motor_running
	pop bc
	ret
;
;	Disk seek
;
henelec_seek:
	call wait_ready
	ld c,#DATAREG
	ld a,(_hfdc_track)
	cpl
	ld b,a
	call henelec_out
	ld bc,#(SEEKCMD*256)+CMDREG
	call henelec_out
swait:
	in a,(piob_data)
	and #0x40
	jr z,swait
	ld c,#STAREG
	call henelec_in
	and #0x98
	ld l,a
	; check for a timeout
	ret
;
;	Disk read
;	B = sector
;	HL = buffer
;
;	Length is taken from the media and I'm not sure there
;	is time to do better!
;
henelec_diskread:
	call wait_ready
	ld c,#SECREG
	call henelec_out
	ld bc,#(READCMD * 256)+CMDREG
	call henelec_out
	ld bc,#(DATREG*256)+DATREG-4
	ld a,b
	out (piob_data),a
rdwait: in a,(piob_data)		; Check status
	and #0xC0
	jp z,rdwait
	jp p,rdirq
	ld a,c
	out (piob_data),a
	in a,(pioa_data)
	cpl
	ld (hl),a
	ld a,b
	out (piob_data),a
	inc hl
	jp rdwait
rdirq:	ld c,#STAREG
	call henelec_in
	; check timeout etc
	ret
;
;	Disk write
;	B = sector
;	HL = buffer
;
;	Same basic idea
;
henelec_diskwrite:
	call wait_ready
	ld c,#SECREG
	call henelec_out
	ld bc,#(WRITECMD*256)+CMDREG
	call henelec_out
	ld a,#0xff
	out (pioa_ctrl),a
	out (pioa_ctrl),a
	ld a,b
	out (piob_data),a
	ld bc,#(DATREG*256)+DATREG-8
wbytel:	ld a,(hl)
	inc hl
	cpl
	out (pioa_data),a
wrwait:	in a,(piob_data)
	and 0xC0
	jp z, wrwait
	jp p,wrirq
	ld a,c
	out (piob_data),a
	ld a,b
	out (piob_data),a
	jp wbytel
wrirq:
	ld c,#STAREG
	call henelec_int
	; check timeout etc
	ret
;
;	Fuzix glue
;
_hfd_io:
	ld c, #TRKREG
	call henelec_in
	ld a,(_hfd_track)
	cp b
	jr z,no_seekw
	call henelec_seek
	jr nz,seek_fail
no_seekw:
	ld hl,(_hfd_buffer)
	ld bc,(_hfd_seccmd)	; loads sector into B
	ld a,(_io_page)
	or a
	call nz, map_process_a
	or a
	jr z, iowrite
	call henelec_diskread
iodone:
	call map_kernel
seek_fail:
	ld l,a			; return the status code
	ret
iowrite:
	call henelec_diskwrite
	jr iodone

_hfd_reset:
	call init_pio
	call henelec_reset
	ld bc,#0x5500+SECREG
	call henelec_out
	ld c,#SECREG
	call henelec_in
	ld a,#0x55
	cp b
	ld hl,#0
	ret z
	dec hl
	ret

_hfd_restore:
	ld c, #(CMD_RESTORE * 256) + CMDREG
	call henelec_out
	jr swait
