;
;	WD1793 floppy controller routines, based on the Amrpo 177x driver
;
;	TODO
;	- set step rate option
;	- C glue code
;

	.module nasfdc
	.area _COMMONMEM

	.globl	map_kernel
	.globl	map_process_always
	.globl  _wd_map

FD_CMD		.equ	0xE0
FD_WTRACK	.equ	0xE1
FD_WSECTOR	.equ	0xE2
FD_WDATA	.equ	0xE3
FD_STATUS	.equ	0xE0
FD_RTRACK	.equ	0xE1
FD_RSECTOR	.equ	0xE2
FD_RDATA	.equ	0xE3
FD_DRIVE	.equ	0xE4
; 0-3 drive select bits
; 4 side select (if jumpered)
; 5 reads 1 if LK3 clear write low stop motor write high motor 10 secs
; 6 low single density
; 7 reads 1 if LK4 clear
FD_WAITR	.equ	0xE5
; 7 high on DRQ
; 1 high on not ready
; 0 high on INTRQ
; rest always low

;
;	The Nascom FDC design is very clever. The E5 port will be 0 until
;	the driver needs to act. This allows the system to handle DD 8" (HD
;	3.5") on a 4MHz processor - just about.
;

;
;	Execute a command. HL holds the 
;	where next pointer for which
;	completion is required
;
wd_execute:
	push	bc
	in	a,(FD_WAITR)
	and	#0x02
	call	nz, start_motor
	pop	bc
	ld	a,b
	ld	bc,#FD_WAITR
	di
	out	(FD_CMD),a
	jp	(hl)
;
;	We don't have the clocks to handle wrong sector sizing, so be
;	warned.
;
wd_rx:
	ex	de,hl
wait_rdrq:
	;	The timing here is crazy tight for HD and this relies on the
	;	non taken jr being fast, and that fact in r,(c) sets flags
	; 	but in a,(n) does not
	in	a,(c)
	jr	z,wait_rdrq
	in	a,(FD_RDATA)		; We must grab the data before we
					; error check
	jp	m, wait_rdrq
	in	a,(FD_STATUS)
	;	Error bits set ?
	and	#0x0C
wd_out:
	push	af
	ld	a,(_int_disabled)
	or	a
	jr	nz, no_ei
	ei
no_ei:
	pop	af
	ret
	;	NZ = error
wd_nodata:
	in	a,(FD_WAITR)
	rra
	jr	c, wd_nodata;		; TODO - timeout
	in	a,(FD_STATUS)
	or	a
	jr	wd_out

wd_tx:
	ex	de,hl
wait_wdrq:
	in	a,(c)
	jr	z,wait_wdrq
	out	(FD_WDATA),a
	jp	m, wait_wdrq
	bit	1,a
	ret	nz
	;	Check right error bits of write
	and	#0x0C
	jr	wd_out

;
;	Forcibly issue a reset. This is a special case
;	so we handle it directly
;
wd_bonk:
	ld	a,#0xD0		; INTRQ
	out	(FD_CMD),a
	call	wait50ms
	ret

wd_wait:
	push	hl
	push	de
	ld	hl,#0x8000
	ld	e,#5	; reset cycles
wd_waitl:
	in	a,(FD_STATUS)
	rra
	jr	nc, wd_notbsy
	dec	hl
	ld	a,h
	or	l
	jr	nz, wd_waitl
	; Timed out. Issue a D0 and try again
	call	wd_bonk
	dec	e
	jr	nz, wd_waitl
	; We failed. Controller has left for lunch
	pop	de
	pop	hl
	ret
wd_notbsy:
	pop	de
	pop	hl
	xor	a
	ret
;
;	Higher level operations
;

;
;	Restore the disk to track 0. Returns the error bits
;
	.globl	_wd_restore

_wd_restore:
	call	wd_wait
	ld	b,#0x08		; Restore
	ld	hl,#wd_nodata
	call	wd_execute
	ld	hl,#1		; C error return
	ret	nz		; Command error
	and	#0x10
	ret	nz		; Seek error
wd_rx_exec_ret:
	ld	hl,#wd_rx
wd_exec_ret:			; wd_execute
	call	wd_execute	; and set C return
	ld	l, a
	jp	map_kernel	; ensure final map is correct

;
;	Seek the disk to the desired track. Use an address
;	read to ensure we are spun up
;
	.globl	_wd_seek

_wd_seek:
	call	wd_wait
	ld	a,l
	out	(FD_WTRACK),a
	ld	b,#0x1E		; 20ms step (very conservative) - want a way
				; to set this TODO
	ld	hl,#wd_nodata
	jr	wd_exec_ret

;
;	Prepare for a read or write
;	H = track L = sector
;
;	TODO: caller needs to own setting the side bits
;
	.globl	_wd_setup

_wd_setup:
	call	wd_wait
	ld	a,h
	out	(FD_WTRACK),a
	ld	a,l
	out	(FD_WSECTOR),a
	ret
;
;	Disk read, HL is buffer
;
	.globl	_wd_read

_wd_read:
	ld	a,(_wd_map)
	or	a
	call	nz, map_process_always
	call	wd_wait
	ex	de,hl		; data into DE
	ld	b,#0x84		; READ
	jr	wd_rx_exec_ret
;
;	Write, HL is buffer
;
	.globl	_wd_write

_wd_write:
	ld	a,(_wd_map)
	or	a
	call	nz, map_process_always
	call	wd_wait
	ex	de,hl
	ld	b,#0xA4		; WRITE
	ld	hl,#wd_tx
	jr	wd_exec_ret
;
;	Platform specific routines
;
;	Assumes track register makes sense sort of
;
start_motor:
	in	a,(FDC_DRIVE)
	and	#0xDF
	out	(FDC_DRIVE),a
wait_motor:
	in	a,(FD_WAITR)
	bit	1,a
	ret	z
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	dec	bc
	ld	a,b
	or	c
	jr	nz, wait_motor	
	inc	a		; force NZ
	ret
	

wait50ms:
	; 50 ms at 4MHz is 200000 cycles
	;
	ld	a,#12		; 1212 loops
spin:
	ld	b,#101
spin2:
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	djnz	spin2
	dec	a
	jr	nz,spin
	ret

	.area	_COMMONDATA

_wd_map:
	.byte	0
