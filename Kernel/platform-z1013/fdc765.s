;
;		765 Floppy Controller Support
;
;	This is based upon the Amstrad NC200 driver by David Given
;
;	TODO
;	Timing checks
;	Motor
;	Initialize drive step rate etc (we rely on the firmware for now)
;	Step rate
;	Head load/unload times
;	Write off time	af
;	(12ms step 30ms head stabilize, 4ms head load, max (0xf) head
;	unload)
;
		.module fdc765

		.include "kernel.def"
		.include "../kernel-z80.def"

	.globl  map_process_always
	.globl	map_kernel

	.globl _fd765_do_nudge_tc
	.globl _fd765_do_recalibrate
	.globl _fd765_do_seek
	.globl _fd765_do_read
	.globl _fd765_do_write
	.globl _fd765_do_read_id
	.globl _fd765_motor_on
	.globl _fd765_motor_off

	.globl _fd765_track
	.globl _fd765_head
	.globl _fd765_sector
	.globl _fd765_status
	.globl _fd765_buffer
	.globl _fd765_is_user
	.globl _fd765_sectors
	.globl _fd765_drive

	.globl _vtborder

	.globl diskmotor

	.area _COMMONMEM

;
_fd765_do_nudge_tc:
	out	(0xF8),a		; Any access
	ret

; Writes A to the FDC data register.

fd765_tx:
	push	bc
	ex	af, af'
fd765_tx_loop:
	in	a, (0xF0)
	add a
	jr	nc, fd765_tx_loop
	add	a
	jr	c, fd765_tx_exit	; controller doesn't want data ??
	ex	af, af'
	out	(0xF1), a
	ex	(sp),hl
	ex	(sp),hl
fd765_tx_exit:
	pop	bc
	ret

; Reads bytes from the FDC data register until the FDC tells us to stop (by
; lowering DIO in the status register).

fd765_read_status:
	ld	hl, #_fd765_status
read_status_loop:
	in	a, (0xF0)
	rla 				; RQM...
	jr	nc, read_status_loop 	; ...low, keep waiting 
	rla				; DIO...
	ret	nc			; ...low, no more data
	in	a,(0xF1)			;
	ld	(hl),a
	inc	hl
	ex	(sp),hl			; wait for the 765A
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	jr	read_status_loop	; next byte
_fd765_status:
	.ds 8				; 8 bytes of status data

; Sends the head/drive byte of a command.

send_head:
	ld	hl, (_fd765_head)	; l = head h = drive
	ld	a, l
	add	a
	add	a
	add	h
	jr	fd765_tx

; Performs a RECALIBRATE command.

_fd765_do_recalibrate:
	ld	a, #0x07		; RECALIBRATE
	call	fd765_tx
	ld	a, (_fd765_drive)	; drive #
	call	fd765_tx
	jr	wait_for_seek_ending

; Performs a SEEK command.

_fd765_do_seek:
	ld	a,#0x0f			; SEEK
	call	fd765_tx
	call	send_head		; specified head, drive #0
	ld	a, (_fd765_track)	; specified track
	call	fd765_tx
	jr	wait_for_seek_ending
_fd765_track:
	.db	0
_fd765_sector:
	.db	0
;
;	These two must remain adjacent see send_head
;
_fd765_head:
	.db	0
_fd765_drive:
	.db	0

; Waits for a SEEK or RECALIBRATE command to finish by polling SENSE INTERRUPT STATUS.
wait_for_seek_ending:
	ld	a, #0x08		; SENSE INTERRUPT STATUS
	call	fd765_tx
	call	fd765_read_status

	ld	a, (_fd765_status)
	bit	5, a			; SE, seek end
	jr	z, wait_for_seek_ending

	; Now settle the head (FIXME: what is the right value ?)
	ld a,	#30			; 30ms
;
;	FIXME: adjust for our speed - do we just assume 2MHz and hedge ?
;
wait_ms:
	push	bc
wait_ms_loop:
	ld	b,#0xDC		; FIXME: set for 4MHZ
wait_ms_loop2:
	dec	b
	jr	nz, wait_ms_loop2
	dec	a
	jr	nz, wait_ms_loop
	pop	bc
	ret

_fd765_motor_off:
	xor a
	ld (diskmotor),a
	; TODO - how ?
	ret

_fd765_motor_on:
	ld a,(diskmotor)
	or a
	ret nz
	ld a,#0x08
	ld (diskmotor),a
	; Take effect
	call map_kernel
	; Now wait for spin up

	ld e,#10		; FIXME right value ?? 	
wait2:
	; The classic Z80 KHz timing loop
	ld bc,#2000	; 2MHzMHz
wait1:
	dec bc
	ld a,b
	or c
	jr nz, wait1
	dec e
	jr nz, wait2
	ret
;
; Reads a 512-byte sector, after having previously saught to the right track.
;
; We need to be doubly careful here as the 765A has a 'feature' whereby it
; won't report an overrun on the last byte so we must always make timing
;
_fd765_do_read:
	ld a, #0x46			; READ SECTOR MFM

	; FIXME: need to return a last cmd byte here and write it
	; after this crap or we may miss if we write just the sector hits
	; the head (BACKPORT ME ??)
	call setup_read_or_write

	ld a, (_fd765_is_user)
	or a
	push af
	call nz, map_process_always

	di				; performance critical,
					; run with interrupts off

	xor a
	call fd765_tx			; send the final unused byte
					; to fire off the command	
	ld hl, (_fd765_buffer)
	ld c, #0xF1
	ld de, #0x2000			; so we can make timing
	jp read_wait
;
;	First 256 bytes
;
read_loop:
	ini
	dec e
	jp z, read_wait2
read_wait:
	in a,(0xF0)			; read the fdc status
	or a	; FIXME check flags
	jp p, read_wait
	and d
	jp nz, read_loop
	jp read_finished
;
;	Second 256 bytes
;
read_loop2:
	ini
	dec e
	jp z, read_flush_wait
read_wait2:
	in a,(0xF0)			; read the fdc status
	or a	; FIXME ditto
	jp p, read_wait2
	and d
	jp nz, read_loop2
	jp read_finished
;
;	Flush out any extra data (no tc control)
;
read_flush:
	in a,(0xF1)
read_flush_wait:
	in a,(0xF0)
	; FIXME ditto
	jp p, read_flush_wait
	and d
	jp nz, read_flush
;
;	And done
;
read_finished:
	ld (_fd765_buffer), hl
	call _fd765_do_nudge_tc		; Tell FDC we've finished
	ei

	call fd765_read_status

	pop af
	ret z
	jp map_kernel

;
;	Write is much like read just the other direction
;
_fd765_do_write:
					; interrupts off
	ld a, #0x45			; WRITE SECTOR MFM
	call setup_read_or_write

	ld a, (_fd765_is_user)
	or a
	push af
	call nz, map_process_always

	di

	xor a
	call fd765_tx			; send the final unused 0 byte
					; to fire off the command	
	ld hl, (_fd765_buffer)
	ld c, #0xF1
	ld de,#0x2000			; to make timing
	jp write_wait

write_loop:
	outi
	dec e				; FIXME tidy loops (see CP/M)
	jp z, write_wait2
write_wait:
	in a,(0xF0)
	or a
	jp p, write_wait
	and d
	jp nz, write_loop
	jp write_finished
write_loop2:
	outi
	dec e
	jp z, write_flush_wait
write_wait2:
	in a,(0xF0)
	or a
	jp p, write_wait2
	and d
	jp nz, write_loop2
	jp write_finished
write_flush:
	in a,(0xF1)
write_flush_wait:
	in a,(0xF0)
	or a
	jp p, write_flush_wait
	and d
	jp nz, write_flush
write_finished:
	ld (_fd765_buffer), hl
	call _fd765_do_nudge_tc		; Tell FDC we've finished
	ei
	call fd765_read_status

	pop af
	ret z
	jp map_kernel

; Given an FDC opcode in A, sets up a read or write.

setup_read_or_write:
	call fd765_tx			; 0: send opcode (in A)
	call send_head			; 1: specified head, drive #0
	ld a, (_fd765_track)		; 2: specified track
	call fd765_tx
	ld a, (_fd765_head)		; 3: specified head
	call fd765_tx
	ld a, (_fd765_sector)		; 4: specified sector
	ld b, a
	call fd765_tx
	ld a, #2			; 5: bytes per sector: 512
	call fd765_tx
	ld a, (_fd765_sectors)		
	add b				; add first sector
	dec a				; 6: last sector (*inclusive*)
	call fd765_tx
	ld a, #0x2A   			; 7: Gap 3 length (2A is standard for 3" drives)
	call fd765_tx
	; We return with the final unused 0 value not written. We need all
	; the other stuff lined up before we write this.
	ret

_fd765_buffer:
	.dw 0
_fd765_is_user:
	.db 0
_fd765_sectors:
	.db 0

; Read the next sector ID off the disk.
; (Only used for debugging.)

_fd765_do_read_id:
	ld a, #0x4a 				; READ MFM ID
	call fd765_tx
	call send_head				; specified head, drive 0
	jp fd765_read_status
