#include "../../build/kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"
;
;	All the standard 80bus floppy cards (except the Henelec) work
;	the same basic way. They just disagree a bit on drive select
;	and on which address is the status. We do the minimal needed in
;	assembler
;
;	The read and write data loops here are taken from
;	"GM809 and Eight Inch Drives" 80BUS News July-Oct 1982, by D Parkinson
;	who deserves a medal for figuring this one out.
;
;	We are capable of handling 3.5" HD disks
;
	.export _fdc80_readsec
	.export _fdc80_writesec
	.export _fdc80_cmd
	.export _fdc80_seek
	.export _fdc80_reset
	.export _fdc80_mwait

	.export _fdc80_track
	.export _fdc80_dptr
	.export _fdc80_iopage

	.common
;
;	Interrupts are off, any NMI sources are quiet
;
_fdc80_readsec:
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	; DE is now the info word we passed (D is port, E is command)
	ld	c,d
	call	motorbusy_check
	jr	nz, fdcdone
	ld	hl,(_fdc80_dptr)
	ld	a,(_fdc80_iopage)
	or	a
	call	nz, map_proc_a
	ld	a,e
	out	(0xE0),a		; Issue the command
	ex	(sp),hl
	ex	(sp),hl			; Delay
	jr	read_sync
read_loop:
	; LDI is not a win as we'd have to reload C
	in	a,(0xE3)		; 11
	ld	(hl),a			; 7
	inc	hl			; 6
read_sync:
	; From the moment DRQ goes true we have 54 T states to read the
	; data byte
	in	a,(c)			; 12
	jr	z, read_sync		; 12 / 7
	jp	m, read_loop		; 10
	; IRQ or timeout error
read_status:
	call	map_kernel
	call	motor_check
	jr	nz, fdcdone
	; Next byte will be status
	in	a,(0xE0)
	ld	h,0
	ld	l,a			; Return it to l
	pop	bc	
	ret
fdcdone:
	pop	bc			; Return error
	ld	hl,0xFFFF
	ret

;
;	Same idea as rx, we pass in a port/command pair
;
_fdc80_writesec:
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	; DE is now the info word we passed (D is port, E is command)
	ld	c,d
	call	motorbusy_check
	jr	nz, fdcdone
	ld	a,(_fdc80_iopage)
	or	a
	call	nz, map_proc_a
	ld	a,e
	ld	hl,(_fdc80_dptr)
	out	(0xE0),a		; issue command
	ex	(sp),hl
	ex	(sp),hl
write_loop:
	ld	a,(hl)			; 7
	inc	hl			; 6
write_wait:
	; For write DRQ requires data within 46 T states - hence the
	; load of A must occur first
	in	b,(c)			; 12 - sets flags
	jr	z, write_wait
	out	(0xE3),a		; 11 - send data even if errorr
					; we don't have time otherwise
	jp	m, write_loop		; 10 - all good
	jr	read_status

;
;	Motor check
;
motorbusy_check:
	in	a,(0xE0)
	bit	0,a
	ret	nz
motor_check:
	in	a,(c)
	bit 	1,a
	ret

	.code
;
;	Do a seek
;	
_fdc80_seek:
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	; DE is now the info word we passed (D is port, E is command)
	; Step rate is passed in
	ld	c,d
	call	motorbusy_check
	jr	nz, seek_fail
	ld	a,(_fdc80_track)
	out	(0xE3),a		; track we want
issue_cmd:
	ld	a,e
	;	Non data commands
	out	(0xE0),a
	ex	(sp),hl
	ex	(sp),hl
wait_ready:
	in	a,(c)
	rra
	jr	nc, wait_ready
	in	a,(0xE0)		; status byte
	ld	h,0
	ld	l,a
	pop	bc
	ret

_fdc80_cmd:
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	; DE is now the info word we passed (D is port, E is command)
	; Step rate is passed in
	ld	c,d
	call	motorbusy_check
	jr	z, issue_cmd
seek_fail:
	pop	bc
	ld	hl,0xFFFF
	ret

_fdc80_reset:
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	ld	c,e
	ld	a,0xD0
	out	(0xE0),a
	ex	(sp),hl
	ex	(sp),hl
	; TODO wait for reset CHECKME
	; Add timeout
reset_wait:
	in	a,(c)
	bit	1,a
	jr	nz, reset_wait
	pop	bc
	ret

_fdc80_mwait:
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	ld	de,0x2000
mwait:
	in	a,(c)
	and	2
	jr	z, mwdone
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	dec	de
	ld	a,d
	or	e
	jr	nz, mwait
	dec	a		; now -1
mwdone:
	ld	l,a		; 0 or -1
	ld	h,a
	pop	bc
	ret

	.bss
_fdc80_track:
	.byte	0
_fdc80_iopage:
	.byte	0
_fdc80_dptr:
	.word	0
