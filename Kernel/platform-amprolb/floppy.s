;
;	WD1772 floppy controller routines for the 1772 on the Ampro
;

	.module floppy
	.area _COMMONMEM

	.globl	map_kernel
	.globl	map_proc_always
	.globl  _wd_map

USEC_WAIT	.equ	19

FD_CMD		.equ	0xC0
FD_WTRACK	.equ	0xC1
FD_WSECTOR	.equ	0xC2
FD_WDATA	.equ	0xC3
FD_STATUS	.equ	0xC4
FD_RTRACK	.equ	0xC5
FD_RSECTOR	.equ	0xC6
FD_RDATA	.equ	0xC7

;
;	Execute a command. HL holds the 
;	where next pointer for which
;	completion is required
;
wd_execute:
	in	a,(FD_STATUS)
	rla
	call	nc, start_motor
	ld	a,b
	out	(FD_CMD),a
	ld	b,#19
cmwait:	djnz	cmwait
	; 236 cycles later
	jp	(hl)	; so after this over the 60us
;
;	Receive 256 or 512 bytes. Handle surprise 1K sectors
;	by just dumping the remainder.
;
wd_rx256:
	ex	de,hl	; data pointer into HL
	ld	bc,#FD_RDATA
	jr	wait_rdrq2
wd_rx:
	ex	de,hl
	ld	bc,#FD_RDATA
wait_rdrq:
	in	a,(FD_STATUS)
	rra
	jr	nc, read_done
	rra
	jr	nc, wait_rdrq
	;	Read byte
	ini
	jp	nz, wait_rdrq
wait_rdrq2:
	in	a,(FD_STATUS)
	rra
	jr	nc, read_done
	rra
	jr	nc, wait_rdrq2
	;	Read byte
	ini
	jp	nz, wait_rdrq2
	;	Over 512 bytes. Drain
wait_rddrain:
	in	a,(FD_STATUS)
	rra	
	jr	nc, read_done
	rra
	jr	nc, wait_rddrain
	in	a,(FD_RDATA)
	jp	wait_rddrain
read_done:
	bit	1,a
	ret	nz
	;	Error bits set ?
	and	#0x0C
	ret
	;	NZ = error

wd_nodata:
	in	a,(FD_STATUS)
	and	#0x01
	ret	z
	jr	wd_nodata

;
;	If the controller asks for excess data we'll just spew
;	random memory. That should be fine for simple 8bit micros.
;
wd_tx:
	ld	bc,#FD_WDATA
	ex	de,hl
wait_wdrq:
	in	a,(FD_STATUS)
	rra
	jr	nc, write_done
	rra
	jr	nc, wait_wdrq
	;	Write byte
	outi
	jp	wait_wdrq
write_done:
	bit	1,a
	ret	nz
	;	Check right error bits of write
	;	fold int read_done ?
	and	#0x0C
	ret

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
	.globl _wd_restore

_wd_restore:
	call	wd_wait
	ld	b,#0x08		; Restore
	ld	hl,#wd_nodata
	call	wd_execute
	ld	hl,#1		; C error eturn
	ret	nz
	call	wait50ms
	ld	b,#0xC8
	ld	de,#scratch
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
	.globl _wd_seek

_wd_seek:
	call	wd_wait
	ld	a,l
	out	(FD_WTRACK),a
	ld	b,#0x1C
	ld	hl,#wd_nodata
	jr	wd_exec_ret

;
;	Prepare for a read or write
;	H = track L = sector
;
	.globl _wd_setup

_wd_setup:
	call	wd_wait
	ld	a,h
	out	(FD_WTRACK),a
	ld	a,l
	out	(FD_WSECTOR),a
	ret
;
;	512 byte read, HL is buffer
;
	.globl _wd_read

_wd_read:
	ld	a,(_wd_map)
	or	a
	call	nz, map_proc_always
	call	wd_wait
	ex	de,hl		; data into DE
	ld	b,#0x88		; READ
	jr	wd_rx_exec_ret
;
;	Write, HL is buffer
;
	.globl _wd_write

_wd_write:
	ld	a,(_wd_map)
	or	a
	call	nz, map_proc_always
	call	wd_wait
	ex	de,hl
	ld	b,#0xA8		; WRITE
	ld	hl,#wd_tx
	jr	wd_exec_ret


;
;	Platform specific routines
;
;	Assumes track register makes sense sort of
;
start_motor:
	push	bc
	push	de
	push	hl
	call	wd_wait
	xor	a
	in	a,(FD_RTRACK)
	out	(FD_WDATA),a
	ld	a,#0x18		; Seek, no verify
	out	(FD_CMD),a	; step doesn't matter here
	call	wait_second
	pop	hl
	pop	de
	pop	bc
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
;
;	We have interrupt driven I/O so we can chill here. The swap
;	case needs tricks cleaning up for any swap interrupt off mess
;	not this code hacking about.
;
wait_second:
	ld	b,#20
waiter:
	push	bc
	call	wait50ms
	pop	bc
	djnz	waiter
	ret
	
_wd_map:
	.byte	0

; Space for the C8 command data and anything else we need

scratch:
	.ds	16
