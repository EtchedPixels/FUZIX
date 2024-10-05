;
;	GM849 SASI/SCSI
;
;	The controller handles /ACK generation for us but the rest is our
;	job. The 849 can in theory multi master but we don't handle that.
;

	.export _gm849_cmd

	.common

gm849_sasi_w:
	in	a,(0xE5)		; Check phase is as expected
	and	0x0E
	cp	e
	ret	nz
	ld	a,(hl)
	out	(0xE6),a		; Put on the bus, will do /ACK for us
	dec	bc
	ld	a,b
	or	c
	jr	nz, gm849_sasi_w
	ret

gm849_sasi_r:
	in	a,(0xE5)		; Same idea for read
	and	0x0E
	cp	e
	ret	nz
	in	a,(0xE6)
	ld	a,(hl)
	dec	bc
	ld	a,b
	or	c
	jr	nz, gm849_sasi_r
	ret

;
;	Figure out how to map user pages if needed
;
set_map:
	ld	a,(_td_raw)
	or	a
	ret	z
	dec	a
	jp	z, map_proc_always
	ld	a,(_td_page)
	jp	map_for_swap
	

	; No multimastering so simple
_gm849_cmd:
	push	bc
	xor	a
	out	(0xE5),a	; bus quiet
	ld	a,(_sasi_mask)
	out	(0xE6),a	; put our selection mask on the bus
	ld	a,0x02
	out	(0xE5),a	; drive \SEL
	ld	a,(_sasi_mask)
	or	0x80		; FIXME: hardcoding us as 7
	out	(0xE6),a
	
	ld	de,0x4000
selwait:			; Target will assert busy if present
	dec	de
	ld	a,d
	or	e
	jr	z, timeout
	in	a,(0xE5)
	bit	5,a
	jr	nz, selwait

	ld	a,0x00
	out	(0xE5),a	; Drop \SEL now \BUSY is low

	; Target responded
xfer_loop:
	in	a,(0xE5)
	bit	4,a		; Target went away
	jr	z, fell_off_bus
	bit	3,a
	jr	z, bad_xfer	; We don't support or expect MSG
	and	#0x06
	ld	e,a		; Save for phase change check
	jr	z, tx_data
	cp	4
	jr	z, tx_cmd
	ld	hl,(_sasi_dptr)
	cp	2
	jr	z, rx_data
	; So it's status
	ld	hl,_scsi_status
	ld	bc,2
	call	gm849_sasi_r
	pop	bc
	ld	hl,0
	ret
tx_cmd:
	ld	hl,_sasi_cmdbuf
	ld	bc,(_sasi_cmdlen)
	call	gm849_sasi_w
	jr	xfer_loop
rx_data:
	ld	a,(_sasi_fast)
	or	a
	jr	nz, rx_slow
	; This is a bit of a hack. We do disk block ops via an optimized
	; path as we can be (fairly) sure the device won't wander off
	; or shortcut the transfer
	call	set_map
	ld	bc,#0xE6
	inir
	inir
	call	map_kernel
	jr	xfer_loop
rx_slow:
	ld	bc,(_sasi_datalen)
	call	set_map
	call	gm849_sasi_r
	push	af
	jr	nz, phase
	call	map_kernel
	jr	xfer_loop
tx_data:
	ld	a,(_sasi_fast)
	or	a
	jr	nz, tx_slow
	call	set_map
	ld	bc,#0xE6
	otir
	otir
	call	map_kernel
	jr	xfer_loop
tx_slow:
	ld	bc,(_sasi_datalen)
	call	set_map
	call	gm849_sasi_w
	jr	nz, phase
	call	map_kernel
	jr	xfer_loop
phase:
	pop	bc
	ld	hl,1
	ret
bad_xfer:
	pop	bc
	ld	hl,2
	ret
timeout:
	pop	bc
	ld	hl,3
	ret
fell_off_bus:
	pop	bc
	ld	hl,4
	ret
