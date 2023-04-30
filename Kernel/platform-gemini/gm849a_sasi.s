	.module gm8x9_sasi
;
;	SASI/SCSI support for the GM829/49/49A
;	This is basically a dumb bus interface except that it automates
;	ACK handling.

	.include "kernel.def"
	.include "../kernel-z80.def"

	.globl _si_read
	.globl _si_write
	.globl _si_writecmd
	.globl _si_select
	.globl _si_reset
	.globl _si_clear
	.globl _si_deselect

	.globl _io_page
	.globl _si_dcb

	.globl map_process_a
	.globl map_kernel

	.area _COMMONMEM
;
;	Wait for /REQ. Preserve registers except for A
;	Z = ok, NZ = error, in which case L holds any error info and is != 0
;
;	FIXME: add timeout and error checks
;
si_waitreq:
	ld l,#1
si_waitreql:
	in a,(0xE5)
	; What should we do about monitoring BSY, C/D and I/O direction ?
	bit 0,a
	jr nz, si_waitreql
	ld l,#0
	ret
;
;	Read a block of bytes into HL
;
_si_read:
	ld c,#0xE6		; port
	ld de,(_si_dcb + 16)	; length
	ld a,(_io_page)
	or a
	call nz, map_process_a
si_readw:
	call si_waitreq
	jr nz, si_bad
	;
	; Each time we see /REQ we write a byte which causes the controller
	; to generate /ACK for us
	;
	ini
	dec de
	ld a,d
	or e
	jr z, si_readw
si_good:
	ld hl,#0
si_bad:
	jp map_kernel

_si_write:
	ld b,c
	ld c,#0xE6
	ld de,(_si_dcb+16)	; length word
	ld a,(_io_page)
	or a
	call nz, map_process_a
si_writew:			; wait for REQ
	call si_waitreq
	jr nz, si_bad
	outi
	dec de
	ld a,d
	or e
	jr nz, si_writew
	jr si_good

	.area _CODE
;
;	On entry HL is the length of the command block
;
_si_writecmd:
	in a,(0xE5)
	; FIXME: check errors/timeout
	bit 0,a
	jr nz, _si_writecmd	; wait for REQ
	bit 2,a
	jp nz, si_bad		; phase wrong
	ex de,hl
	ld hl,#_si_dcb
	jp si_writew
;
;	Select a device
;
_si_select:
	ld a,(_si_dcb + 20)
	cp #7
	jr z, si_badsel		; clash of id
;
;	Now perform the device selection ritual
;
	ld b,a
	ld a,#0x80
shiftid:
	rra
	djnz shiftid

	ld de,#0xFFFF
selwait:
	dec de
	ld a,d
	or e
	jr z, timedout
	; FIXME errors and timeout
	in a,(0xE5)
	; Wait for BSY to drop so the bus is idle
	bit 4,a
	jr z, selwait
	; Now put the mask on the bus and assert /SEL
	ld a,#0x05
	out (0xE5),a
selwait2:
	dec de
	ld a,d
	or e
	jr z, timedout
	; FIXME: errors, timeout
	in a,(0xE5)
	bit 4,a
	jr nz, selwait2
	; Drive responded, drop /SEL
	ld a,#0x07
	out (0xE5),a
	ld hl,#0
	ret
timedout:
	ld hl,#1
	ret
si_badsel:
	ld hl,#2
	ret

_si_reset:
	; Assert reset
	ld a,#0x03
	out (0xE5),a
	; FIXME: check spec for delay if needed
	ld a,#0x07
	out (0xE5),a
	ret
_si_clear:
	; Disconnect from the bus
	ld a,#0x0F
	out (0xE5),a
_si_deselect:
	ret

