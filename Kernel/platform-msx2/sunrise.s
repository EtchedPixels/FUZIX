;
;	Sunrise style IDE
;

	.globl _do_ide_init_drive
	.globl _do_ide_begin_reset
	.globl _do_ide_end_reset
	.globl _do_ide_flush_cache
	.globl _do_ide_xfer

	.globl _ide_error
	.globl _ide_base
	.globl _ide_addr
	.globl _ide_lba
	.globl _ide_is_read

	.globl _mapslot_bank1
	.globl _slotram
	.globl _ide_slot

	.globl _ticks
	.globl _devide_buf


	.globl _int_disabled

	.module sunrise

IDE_REG_ERROR		.equ	1
IDE_REG_FEATURES 	.equ	1
IDE_REG_SEC_COUNT	.equ	2
IDE_REG_LBA_0		.equ	3
IDE_REG_LBA_1		.equ	4
IDE_REG_LBA_2		.equ	5
IDE_REG_LBA_3		.equ	6
IDE_REG_DEVHEAD		.equ	6
IDE_REG_COMMAND		.equ	7
IDE_REG_STATUS		.equ	7

; For Sunrise at least
IDE_REG_CONTROL		.equ	14


IDE_ERROR		.equ	0
IDE_BUSY		.equ	7

IDE_STATUS_READY	.equ	0x40
IDE_STATUS_DATAREQUEST	.equ	0x08

IDE_CMD_READ_SECTOR	.equ	0x20
IDE_CMD_WRITE_SECTOR	.equ	0x30
IDE_CMD_FLUSH_CACHE	.equ	0xE7
IDE_CMD_IDENTIFY	.equ	0xEC

BLK_OP_ADDR		.equ	0
BLK_OP_ISUSER		.equ	2
BLK_OP_LBA		.equ	6
BLK_OP_ISREAD		.equ	12


	.area _CODE1

devide_wait_ready:
	xor a
	ld (_ide_error),a
	ld a,#IDE_STATUS_READY
devide_wait:
	push de
	push hl
	ld c,a
	ld de,(_ticks)
wait_nbusy:
	ld hl,(_ticks)
	or a
	sbc hl,de
	bit 2,h			; We spin fast enough this is a safe test
	jr nz, wait_timeout
	ld a,IDE_REG_STATUS(ix)
	bit IDE_BUSY,a
	jr nz, wait_nbusy
	; Check for an error
	bit IDE_ERROR,a
	jr z, noerror
	; Keep NZ set
	ld (_ide_error),a
	ld a,IDE_REG_ERROR(ix)
	ld (_ide_error + 1),a
	jr waitout
noerror:
	; Now see if it's a good status
	and c
	cp c
	jr nz, wait_nbusy
	; We have !busy, !error and the value we wanted - done
waitout:
	pop hl
	pop de
	ret		; Z = good
wait_timeout:
	ld a,#255		; NZ is set already
	ld (_ide_error),a	; save the value
	; NZ set already
	jr waitout

;
;	Flip I/O maps
;
map_sunrise_k:
	push af
	push hl
	ld a,(#_ide_slot)
	di
	call _mapslot_bank1
	jr keepdi
map_sunrise_u:
	push af
	push hl
	ld a,(#_slotram)
	di
	call _mapslot_bank1
	ld a,(_int_disabled)
	or a
	jr nz, keepdi
	ei
keepdi:
	pop hl
	pop af
	ret
;
;	Caller passes  L = disk, (devide_buf) = a tmpbuf in common space
;
_do_ide_init_drive:
	push ix
	ld ix,(_ide_base)
	call map_sunrise_k
	ld IDE_REG_DEVHEAD(ix),l
	ld hl,#0
	call devide_wait_ready
	jr nz, timeout
	ld IDE_REG_COMMAND(ix),#IDE_CMD_IDENTIFY
	ld a,#IDE_STATUS_DATAREQUEST
	call devide_wait
	jr nz, timeout
	call devide_rx_buf
	; returns the tmpbuf to the caller to free
timeout:
	ld a,#'R'
	call map_sunrise_u
	pop ix
	ret

_do_ide_begin_reset:
	push ix
	ld ix,(_ide_base)
	call map_sunrise_k
	ld a,#0x01
	ld (0x4104),a
	ld IDE_REG_DEVHEAD(ix),#0xE0
	ld IDE_REG_CONTROL(ix),#0x06	; start reset (bit 2)
	call map_sunrise_u
	pop ix
	ret

_do_ide_end_reset:
	push ix
	ld ix,(_ide_base)
	call map_sunrise_k
	ld IDE_REG_CONTROL(ix),#0x02
	call map_sunrise_u
	pop ix
	ret

_do_ide_flush_cache:
	push ix
	ld ix,(_ide_base)
	call map_sunrise_k
	ld IDE_REG_LBA_3(ix),l
	ld hl,#0
	call devide_wait_ready
	jr nz, flush_bad
	ld IDE_REG_COMMAND(ix),#IDE_CMD_FLUSH_CACHE
	call devide_wait_ready
	jr z,flush_good
flush_bad:
	dec hl		; to -1
flush_good:
	call map_sunrise_u
	pop ix
	ret

_do_ide_xfer:
	push ix
	ld ix,(_ide_base)
	call map_sunrise_k
	ld e,l
	ld hl, (_ide_lba + 2)
	ld a,h
	and #0x0F			; Merge drive and bits 24-27
	or e
	ld IDE_REG_LBA_3(ix),a
	push hl
	ld hl,#0
	call devide_wait_ready
	pop hl
	jr nz, xfer_timeout
	ld IDE_REG_LBA_2(ix),l
	ld hl, (_ide_lba)
	ld IDE_REG_LBA_1(ix),h
	ld IDE_REG_LBA_0(ix),l
	ld IDE_REG_SEC_COUNT(ix),#1
	ld a,(_ide_is_read)
	or a
	jr z, send_cmd
	ld IDE_REG_COMMAND(ix),#IDE_CMD_READ_SECTOR
	ld a,#IDE_STATUS_DATAREQUEST
	call devide_wait
	jr nz, xfer_timeout
	call devide_xfer_r
	ld hl,#0
	call map_sunrise_u
	pop ix
	ret
send_cmd:
	ld IDE_REG_COMMAND(ix),#IDE_CMD_WRITE_SECTOR
	ld a,#IDE_STATUS_DATAREQUEST
	call devide_wait
	jr nz, xfer_timeout
	call devide_xfer_w
	call devide_wait_ready
	jr nz, xfer_timeout
	ld hl,#0
	call map_sunrise_u
	pop ix
	ret
xfer_timeout:
	ld hl,#0xffff
	call map_sunrise_u
	pop ix
	ret

;
;	For now deal with Sunrise style. If we find any others we can
;	deal with it here
;
devide_xfer_r:
	ld de,(_ide_addr)
	; We don't have to worry about swap being special for this port
	; Our caller also is responsible for working out when to bounce
xfer_do:
	; FIXME: for non Sunrise we may need to change this and the xfer_w
	ld hl,#0x7C00
	ld bc,#0x0200
	ldir
	ret

devide_xfer_w:
	ld hl,(_ide_addr)
	; We don't have to worry about swap being special for this port
	; Our caller also is responsible for working out when to bounce
	ld de,#0x7C00
	ld bc,#0x0200
	ldir
	ret

devide_rx_buf:
	ld hl,(_devide_buf)
	push hl
	ex de,hl
	call xfer_do
	pop hl
	ret

