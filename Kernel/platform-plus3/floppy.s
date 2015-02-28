;
;	ROM timings are used for the drives.
;
;	0x0A, motor on
;	0x32, motor off
;	0xAF, write off
;	0x1E, head settle	ms
;	0x0C, step rate		ms
;	0x0F, head unload
;	0x03, head load x 2 + 1
;
;
	.area _COMMONMEM

	.globl _fdc_motoron, _fdc_motoroff
	.globl _fdc_read, _fdc_write, _fdc_recal, _fdc_seek
	.globl _fdc_user, _fdc_addr
	.globl _fdc_statbuf
	.globl _seekcmd
	.globl _rwcmd
	.globl _recalcmd
	.globl port_map
	.globl map_kernel, map_process_always

fdc_cmd:
	dec b
	jr z, fdclast
fdc_cmdl:
	call fdc_outcmd
	inc hl
	ret c			; Error
	djnz fdc_cmdl
fdclast:
	di
fdc_outcmd:
	push bc
	ld bc, #0x2ffd
	ld e, (hl)
fdc_outcmdw:			; Wait non busy
	in a, (c)
	add a, a
	jr c, fdc_outcmdw
	add a, a		; FIXME: can we use ret m or similar here ?
	ret c			; Failed
	ld b, #0x3F
	out (c), e		; Output the byte
	ex (sp), hl
	ex (sp), hl		; Nap
	pop bc
	ret			; NC

fdc_cmdq:			; Quick (non data) command
	call fdc_outcmd
	djnz fdc_cmdq
	ret

fdc_status:
	ld bc, #0x2ffd
	ld hl, #_fdc_statbuf
fdc_statw:
	in a, (c)
	add a, a
	jr c, fdc_statw
	add a, a
	ret nc			; Finished copying block
	ld b, #0x3F
	ini
	ld b, #0x2F
	ex (sp), hl
	ex (sp), hl
	jr fdc_statw

fdc_statusa:
	call fdc_status
	ld a, (_fdc_statbuf)
	ret

fdc_read:
	ld hl, #_rwcmd
	ld (hl), #66
	ld b, #9
	call fdc_cmd		; Will return with DI
fdc_readgo:
	ld bc, #0x2ffd
	ld d, #0x20
	jr fdc_rwait
;
;	This would be faster using EXX. May be worth looking at depending
;	upon clock counts.
;
fdc_rbyte:
	ld b, #0x3F		; 3FFD
	ini
	ld b, #0x2f
fdc_rwait:			; wait for ready
	in a, (c)
	jp p, fdc_rwait
	and d
	jp nz, fdc_rbyte
	; now get status
	jr fdc_statusa

fdc_write:
	ld hl, #_rwcmd
	ld (hl), #65
	ld b, #9
	call fdc_cmd		; Will return with DI
fdc_writego:
	ld bc, #0x2ffd
	ld d, #0x20
	jr fdc_wwait
fdc_wbyte:
	ld b, #0x40
	outi
	ld b, #0x2f
fdc_wwait:
	in a, (c)
	jp p, fdc_wwait
	; now read status
	jr fdc_statusa

;
;	Seek is interesting as we have to second guess the delays
;
fdc_seek:
	ld (_seekcmd + 2), a		; Cylinder we want
fdc_seek2:
	ld b, a
	ld b, #3
	call fdc_cmdq
	ld bc, (_seekcmd + 2)
	ld a, (curtrack)		; Current track FIXME
	sub c
	bit 7, a
	jr z, seekin
	neg
seekin:				; milliseconds of time for the seek
	add a, a
	ld b, a
	add a, a
	add a, b
	add a, a		; 12ms step rate assumed
	add a, #0x1e		; head settle
	call msdelay
	; We should now get a sense interrupt
	call fdc_sense
	; Check if it worked
	bit 6, a
	jr nz, fdc_seekfail
	ld a, (_seekcmd + 2)
	ld (curtrack), a	; FIXME per drive
	ret			; Z
fdc_seekfail:
	ld a, #0xFF
	ld (curtrack), a	; Mark as busted
	ret			; NZ

fdc_recalibrate:
	ld hl, #_recalcmd
	ld b, #2		; recalibrate, unit
	call fdc_cmdq
	ld a, #80		; or 40 if 40 track !
	ld b, #12		; 12ms
fdc_recald:
	ld a, #80		; or 40 if 40 track !
	call msdelay
	djnz fdc_recald
	ld a, #0x1e		; settle
	call msdelay
	call fdc_status
	bit 6, a
	jr nz, fdc_seekfail
	xor a
	ld (curtrack), a
	ret			; Z

fdc_sense:
	ld hl, #sensecmd
	call fdc_outcmd
	call fdc_statusa
	bit 7, a
	jr nz, fdc_sense
	ld a, (_fdc_statbuf)
	ret

fdc_cmdwait:
	call fdc_sense
	and #0xC0
	cp #0x80
	jr nz, fdc_cmdwait
	ret

;fdc_getunit:
;	ld hl, #fdc_guscmd
;	ld b, #2
;	call fdc_cmdq
;	jr fdc_statusa

;
;	Delay 'a' milliseconds. Trashes A, C.
;
msdelay:			; For uncontended RAM
	ld c, #0xDC
	dec c
	jr nz, msdelay
	dec a
	jr nz, msdelay
	ret
	

;
;	Must be called with the motor timeout stopped
;
_fdc_motoron:
	ld a, (port_map)
	bit 3, a
	ret nz
	or #9
	ld (port_map), a
	ld bc, #0x1ffd
	out (c), a
_fdc_mwait:
	ld bc, #0x3548
	dec bc
	ld a, b
	or c
	jr nz, _fdc_mwait
	ret

_fdc_motoroff:
	ld a, (port_map)
	and #0xF7
	ld (port_map), a
	ld bc, #0x1ffd
	out (c), a
	ret

_fdc_read:
	ld hl, (_fdc_addr)
	ld a, (_fdc_user)
	or a
	push af
	call nz, map_process_always
	call fdc_read
	pop af
	call nz, map_kernel
	ret
	
_fdc_write:
	ld hl, (_fdc_addr)
	ld a, (_fdc_user)
	or a
	push af
	call nz, map_process_always
	call fdc_write
	pop af
	call nz, map_kernel
	ret

_fdc_recal:
	call	fdc_recalibrate
	ld	l, a
	ret

_fdc_seek:
	call	fdc_seek2
	ret

	.area _COMMONDATA

curtrack:
	.byte	0		; FIXME

_rwcmd:
	.byte	0x66		; MFM read / 0x65 for write
	.byte	0		; Drive 0, head 0 (for now)
	.byte	0		; Track
	.byte	0		; Head
	.byte	0		; Sector
	.byte	2		; 512 byte blocks
	.byte	0		; Last sector (== sector)
	.byte	0x2A		; Gap length
	.byte	0xFF

sensecmd:
	.byte	0x08

_seekcmd:
	.byte	0x0F
	.byte	0x00
	.byte	0x00		; Cylinder

_recalcmd:
	.byte	0x07
	.byte	0

_fdc_user:
	.byte	0x00
_fdc_addr:
	.word	0x0000
_fdc_statbuf:
	.word	0,0,0,0
