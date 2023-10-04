;
;	NCR5380 low level. Easier to do this in Z80
;	Design based on the GPL B/P BIOS
;
;	The NCR5380 is basically a bidirectional parallel port with
;	SCSI status lines attached and internal logic to do some of
;	the lowest level signal management. Our job therefore is
;	mostly as a byte shovel in each direction taking care that
;	we deal with the other end changing direction unexpected on us.
;
;	TODO: update to use pseudo-DMA for the transfers
;

		.module nrc5380

		.area _COMMONMEM

	.globl	map_kernel
	.globl  map_proc_always
	.globl  map_for_swap

	.globl  _td_raw
	.globl  _td_page

	.globl	_ncr5380_check

	.globl	_ncr5380_command
	.globl  _ncr5380_reset_on
	.globl	_ncr5380_reset_off

	.globl	_scsi_dbuf
	.globl	_scsicmd
	.globl	_scsimsg
	.globl	_scsi_idbits
	.globl	_scsi_target
	.globl	_scsi_status
	.globl  _scsi_burst

	.include "kernel.def"

PDMA		.equ	0
;	FIXME: tune this
TIMEOUT		.equ	0x8000

	.globl _td_raw

	.globl _ncr5380_command

	.globl _scsimsg
	.globl _scsi_status
	.globl _scsicmd

	.area _DISCARD

_ncr5380_check:
	xor	a
	ld	l,a
	out	(ncr_mod),a
	in	a,(ncr_mod)
	or	a
	ret	z
	inc	a
	inc	a
	out	(ncr_mod),a
	in	a,(ncr_mod)
	dec	a
	dec	a
	ret	z
	dec	l
	ret

	.area _COMMONMEM
;
;	Called with pointers set up and device in C
;
ncr5380_op:
	; Start sane
	xor	a
	out	(ncr_cmd),a
	out	(ncr_mod),a
	out	(ncr_tgt),a

	; Do a select (we don't handle multi-master etc)
	ld	a,(_scsi_idbits)
	or	b
	out	(ncr_data),a
	; These two must be split to avoid glitching the bus
	in	a,(ncr_cmd)
	or	#B_ABUS
	out	(ncr_cmd),a
	ld	a,#B_ASEL|B_ABUS
	out	(ncr_cmd),a
	; The other end of the string should see this and set busy
	; (if there is another end...)
	ld	de,#TIMEOUT

	;
	;	Wait for the device to assert busy. Note that we
	;	don't support multimastering here.
	;
ncr5380_sel_wait:
	; TODO: nicer timeout
	dec	de
	ld	a,d
	or	e
	jr	z, ncr5380_timeout
	in	a,(ncr_bus)
	and	#B_BUSY
	jr	z, ncr5380_sel_wait
	; Drop SEL so we get to see a valid bus request
	; from the target
	xor	a
	out	(ncr_cmd),a

ncr5380_phase:
	in	a,(ncr_bus)
	and	#B_BUSY
	jr	z, ncr5380_exit
	; Clear mode register
	xor	a
	out	(ncr_mod),a
	; Clear interrupt
	in	a,(ncr_int)
.ifne PDMA
	; DMA on
	ld	a,#6
	out	(ncr_mod),a
	ld	c,#ncr_dack
.else
	ld	c,#ncr_data
.endif
	in	a,(ncr_bus)
	and	#B_MSG|B_CD|B_IO
	rra
	rra	; We now have the phase in bits 0-2 for testing
	out	(ncr_tgt),a	; check for match


	; Phase dispatcher
	;
	; The basic idea is that the device goes through a series of
	; phases indicated by signals on the bus. We respond according to
	; the phase by supplying or reciving data.
	;

	ld	e,#1		; Default to non block burst
	ld	hl,#_scsimsg	; Message buffer is used for phase 7
	cp	#7		; Message in
	jr	z, ncr5380_hdin
	ld	hl,(_scsi_dbuf)	; Data buffer is used in data states
	jr	z, ncr5380_hdin
	or	a		; Data out
	jr	z, ncr5380_hdout_block
	dec	a
	jr	z, ncr5380_hdin_block	; Data in
	ld	hl,#_scsicmd	; SCSI cmd block
	dec	a
	jr	z, ncr5380_hdout ; Command out
	ld	hl,#_scsi_status ; Status block
	dec	a
	jr	z, ncr5380_hdin	 ; Status in

ncr5380_timeout:
	;	State error
	ld	l,#1
	ret

.ifne PDMA
;
;	PDMA mode
;
	; Busy dropped

ncr5380_exit:
	xor	a
	out	(ncr_cmd),a
	out	(ncr_tgt),a
	ld	l,a
	ret

	; Things are coming off the bus for us to store
	; We try and keep this loop as tight as we can

ncr5380_hdin_block:
	ld	a,(_scsi_burst)
	; Burst size for transfer. The available buffer must divide by this
	ld	e,a
ncr5380_hdin:
	out	(ncr_dma_r),a	; Value is irrelevant
	ld	d,#0x40
ncr5380_hdin1:
	in	a,(ncr_st)
	ld	b,a
	and	d		; Wait for REQ
	jr	z, ncr5380_hdin2
	ld	b,e
	inir
	jp	ncr5380_hdin
ncr5380_hdin2:
	bit	4,b
	jr	z, ncr5380_hdin1
	jp	ncr5380_phase

	; Same idea other way around

ncr5380_hdout_block:
	ld	a,(_scsi_burst)
	; Burst size for transfer. The available buffer must divide by this
	ld	e,a
ncr5380_hdout:
	ld	a,#B_ABUS
	out	(ncr_cmd),a
	out	(ncr_dma_w),a
	ld	d,#0x40
ncr5380_hdout1:
	in	a,(ncr_st)
	ld	b,a
	and	d		; Wait for REQ
	jr	z, ncr5380_hdout2
	ld	b,e
	otir
	jp	ncr5380_hdout1
ncr5380_hdout2:
	bit	4,b
	jr	z, ncr5380_hdout1
	jp	ncr5380_phase


.else
;
;	PIO mode
;

	; Things are coming off the bus for us to store

ncr5380_hdin_block:
ncr5380_hdin:
	in	a,(ncr_bus)
	bit	5,a		; Wait for REQ
	jr	nz, ncr5380_hdin1
	and	#B_BUSY
	jr	nz, ncr5380_hdin

	; Busy dropped

ncr5380_exit:
	xor	a
	out	(ncr_cmd),a
	out	(ncr_tgt),a
	ld	l,a
	ret

ncr5380_hdin1:
	in	a,(ncr_st)
	and	#B_PHAS
	jr	z, ncr5380_phase
	ini
	ld	a,#B_AACK	; Ack pulse
	out	(ncr_cmd),a
	xor	a
	out	(ncr_cmd),a
	jr	ncr5380_hdin

ncr5380_hdout_block:
ncr5380_hdout:
	ld	a,#B_ABUS
	out	(ncr_cmd),a
	in	a,(ncr_bus)
	bit	5,a		; Wait for REQ
	jr	nz, ncr5380_hdout1
	and	#B_BUSY
	jr	nz, ncr5380_hdout
	jr	ncr5380_exit

ncr5380_hdout1:
	in	a,(ncr_st)
	and	#B_PHAS
	jr	z, ncr5380_phase
	outi
	ld	a,#B_AACK|B_ABUS
	out	(ncr_cmd),a
	xor	a
	out	(ncr_cmd),a
	jr	ncr5380_hdout

.endif

;
;	Perform a SCSI command. The caller loaded the command block
;	into scsicmd, loaded the data pointer into _scsi_dbuf. We will
;	in turn fill in the data as needed or copy it out and then
;	report back with status info
;
;	SCSI command block and the like must be in common. Data need not
;	be.
;
_ncr5380_command:
	ld	a,(_scsi_target)
	ld	b,a
	ld	a,(_td_raw)
	or	a
	jr	z, via_k
	dec	a
	jr	z, via_u
	call	map_for_swap
via_k:
	call	ncr5380_op
	;	will return with L 1 or 0 for bad/good
	jp	map_kernel
via_u:
	call	map_proc_always
	jr	via_k

_ncr5380_reset_on:
	ld	a,#B_RST
	out	(ncr_cmd),a
	ret

_ncr5380_reset_off:
	xor	a
	out	(ncr_cmd),a
	ret

_scsi_status:
	.word	0
_scsi_idbits:		; shifted
	.byte	0
_scsi_target:		; shifted
	.byte	0
_scsimsg:
	.byte	0
_scsi_burst:
	.byte	0
_scsicmd:
	.ds	16
_scsi_dbuf:
	.word	0
