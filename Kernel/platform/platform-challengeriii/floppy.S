;
;	6850/6820 combo as floppy controller
;
;	The OSI systems use the sync hole as a marker and timing tricks
;	to do sector rather than track write. It's a very slow messy
;	approach.
;
;	What they should have done was written metadata with odd parity
;	and data with even parity then they could have started reading
;	anywhere and let the read sync to the start/stop bits. The parity
;	would provide the metadata just as the not quite valid bytes do
;	for conventional encoding
;
	.code

	.export _step_drive

_step_drive:
	pop	de
	pop	hl
	push	hl
	push	de
	push	bc
	; L is the (signed) steps to do
	ld 	a,l
	or	a
	jr	z, step_none
	jp	m,step_out
	ld	b,a
step_in:
	ld	hl,0xC000
	ld	a,0xFB
	ld	(hl),0xFB	; direction set
	nop
	ld	(hl),0xF3	; step
	nop
	ld	(hl),0xFB	; step back
	;	And sleep 5ms
	call	nap5ms
	djnz	step_in
step_none:
	pop	bc
	ret

step_out:
	neg			; positive count
	ld	b,a
step_out_l:
	ld	hl,0xc002
	ld	a,0xFB
	ld	(hl),0xFF	; direction set
	nop
	ld	(hl),0xF7	; step
	nop
	ld	(hl),0xFF	; 
	call	nap5ms
	djnz	step_out_l
	pop	bc
	ret

_set_drive:
	pop	de
	pop	hl
	push	hl
	push	de
	bit	0,l
	ld	hl,0xC000
	jr	z, drive_0
	set	6,(hl)
	ret
drive_0:
	clr	6,(hl)
	ret

_start_motor:
	ld	hl,0xC002
	ld	(hl),0x7F	; motor on
	call	wait_hole
	call	wait_hole
	ret

;
;	Used after we stepped or similar
;	TODO: timeout ?
;
_check_track:
	call	rbyte
	cp	0x57
	jr	nz, _check_track
	; Ok found the track sync byte
	call	rbyte
	cp	l		; track we want
	ret			; Z = good



load_sector:
	; Each sector is a lead of 00
	; A rewrite is timing based so might miss slightly on the data
	; part
	call	rbytec		; read bytes until a control byte arrives
				; long jumps out on seeing three index holes
	cp	#SECHDR
	jr	nz, load_sector
load_sector2:
	call	rbytec
	or	a		; Header padding
	jr	z, load_sector2
	cp	#SECSYNC	; These values never appear as track/sec num
	jr	nz, load_sector
	call	rbytec
	cp	d		; Right track ?
	jr	nz, load_sector
	sub	e		; Compare but set A to 0 if valid
	jr	nz, load_sector
	; GO GO GO
	ld	b,a		; Zero
	ld	a,EVENP
	ld	(ACIA),a
secbyte:
	ld	de,(ACIAC)
	bit	0,e
	jr	z, secbyte
	; should check 4-6 ? do we have time ?
	and	0x70
	jp	nz, error
	ld	(hl),d
	inc	hl
secbyte2:
	ld	de,(ACIAC)
	bit	0,e
	jr	z, secbyte
	; should check 4-6 ? do we have time ?
	and	0x70
	jp	nz, error
	ld	(hl),d
	inc	hl
	djnz	secbyte2

	ld	a,(parity)	; Report if any parity error occurred
	ret

; Everything is in position to write the sector header. We either found the
; previous sector or hole as required,  read that and can now blast our bits
; out


write_sector:
	push	hl
	ld	de,ACIAC
	ld	a,ODDP
	ld	(de),a	
	ld	bc,0x2002	; 8 byte sector header
	ld	hl,sechdr
writel:
	ld	a,(de)
	and	c
	jr	nz, writel
	ld	a,(hl)
	ld	(ACIAD),a
	djnz	writel
	pop	hl
	ld	a,EVENP
	ld	(de),a	
	ld	b,0
writel:
	ld	a,(de)
	and	c
	jr	nz, writel
	ld	a,(hl)
	ld	(ACIAD),a
	djnz	writel
	; Now wait for the final byte to write so we can turn off
	; write/erase
writee:
	ld	a,(de)
	and	c
	jr	z, writee
	; Write off
	; Delay a moment to leave a gap
	; Erase off
	; Hopefully before the next sector
	ret

