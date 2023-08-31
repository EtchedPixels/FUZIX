;
;	NCR5380 low level. Easier to do this in 8085
;	Design based on the GPL B/P BIOS
;
;	The NCR5380 is basically a bidirectional parallel port with
;	SCSI status lines attached and internal logic to do some of
;	the lowest level signal management. Our job therefore is
;	mostly as a byte shovel in each direction taking care that
;	we deal with the other end changing direction unexpected on us.
;
;	The rcbus card has no PDMA hackery so straight PIO
;

	.export	_ncr_command
	.export  _ncr_reset_on
	.export	_ncr_reset_off
	.export _ncr_check

#include "kernel.def"

#define TIMEOUT	0x8000

	.discard

_ncr_check:
	xra	a
	mov	l,a
	out	ncr_mod
	in	ncr_mod
	ora	a
	rz
	inr	a
	inr	a
	out	ncr_mod
	in	ncr_mod
	dcr	a
	dcr	a
	rz
	dcr	l
	ret

	.common
;
;	Called with pointers set up and device in C
;
ncr_op:
	; Start sane
	xra	a
	out	ncr_cmd
	out	ncr_mod
	out	ncr_tgt

	; Do a select (we don't handle multi-master etc)
	lda	_scsi_idbits
	ora	b
	out	ncr_data
	; These two must be split to avoid glitching the bus
	in	ncr_cmd
	ori	B_ABUS
	out	ncr_cmd
	mvi	a,B_ASEL+B_ABUS
	out	ncr_cmd
	; The other end of the string should see this and set busy
	; (if there is another end...)
	lxi	d,TIMEOUT

	;
	;	Wait for the device to assert busy. Note that we
	;	don't support multimastering here.
	;
ncr_sel_wait:
	; TODO: nicer timeout
	dcx	d
	mov	a,d
	ora	e
	jz	ncr_timeout
	in	ncr_bus
	ani	B_BUSY
	jz	ncr_sel_wait
	; Drop SEL so we get to see a valid bus request
	; from the target
	xra	a
	out	ncr_cmd

ncr_phase:
	in	ncr_bus
	ani	B_BUSY
	jz	ncr_exit
	; Clear mode register
	xra	a
	out	ncr_mod
	; Clear interrupt
	in	ncr_int
	in	ncr_bus
	ani	B_MSG+B_CD+B_IO
	rar
	rar	; We now have the phase in bits 0-2 for testing
	out	ncr_tgt	; check for match


	; Phase dispatcher
	;
	; The basic idea is that the device goes through a series of
	; phases indicated by signals on the bus. We respond according to
	; the phase by supplying or reciving data.
	;

	mvi	e,1		; Default to non block burst
	lxi	h,_scsimsg	; Message buffer is used for phase 7
	cpi	7		; Message in
	jz	ncr_hdin
	lhld	_scsi_dbuf	; Data buffer is used in data states
;????	jr	z, ncr_hdin
	ora	a		; Data out
	jz	ncr_hdout_block
	dcr	a
	jz	ncr_hdin_block	; Data in
	lxi	h,_scsicmd	; SCSI cmd block
	dcr	a
	jz	ncr_hdout ; Command out
	lxi	h,_scsi_status ; Status block
	dcr	a
	jz	ncr_hdin	 ; Status in

ncr_timeout:
	;	State error
	mvi	l,1
	ret

;
;	PIO mode
;

	; Things are coming off the bus for us to store

ncr_hdin_block:
ncr_hdin:
	in	ncr_bus
	mov	b,a
	ani	B_REQ		; Wait for REQ
	jnz	ncr_hdin1
	mov	a,b
	ani	B_BUSY
	jnz	ncr_hdin

	; Busy dropped

ncr_exit:
	xra	a
	out	ncr_cmd
	out	ncr_tgt
	mov	l,a
	ret

ncr_hdin1:
	in	ncr_st
	ani	B_PHAS
	jz	ncr_phase
	in	ncr_data
	mov	m,a
	inx	h
	mvi	a,B_AACK	; Ack pulse
	out	ncr_cmd
	xra	a
	out	ncr_cmd
	jmp	ncr_hdin

ncr_hdout_block:
ncr_hdout:
	mvi	a,B_ABUS
	out	ncr_cmd
	in	ncr_bus
	mov	b,a
	ani	B_REQ
	jnz	ncr_hdout1
	mov	a,b
	ani	B_BUSY
	jnz	ncr_hdout
	jmp	ncr_exit

ncr_hdout1:
	in	ncr_st
	ani	B_PHAS
	jz	ncr_phase
	mov	a,m
	out	ncr_data
	inx	h
	mvi	a,B_AACK+B_ABUS
	out	ncr_cmd
	xra	a
	out	ncr_cmd
	jmp	ncr_hdout

;
;	Perform a SCSI command. The caller loaded the command block
;	into scsicmd, loaded the data pointer into _scsi_dbuf. We will
;	in turn fill in the data as needed or copy it out and then
;	report back with status info
;
;	SCSI command block and the like must be in common. Data need not
;	be.
;
_ncr_command:
	lda	_scsi_target
	mov	b,a
	lda	_td_raw
	ora	a
	jz	via_k
	dcr	a
	jz	via_u
	call	map_for_swap
via_k:
	call	ncr_op
	;	will return with L 1 or 0 for bad/good
	jmp	map_kernel
via_u:
	call	map_proc_always
	jmp	via_k

_ncr_reset_on:
	mvi	a,B_RST
	out	ncr_cmd
	ret

_ncr_reset_off:
	xra	a
	out	ncr_cmd
	ret

	.export _scsi_status
	.export _scsi_idbits
	.export _scsi_target
	.export _scsimsg
	.export _scsi_burst
	.export _scsicmd
	.export _scsi_dbuf

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
