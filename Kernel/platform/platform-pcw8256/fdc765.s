;
;		765 Floppy Controller Support
;
;	This is based upon the Amstrad NC200 driver by David Given
;
;	TODO:
;	Check behaviour and timings with CP/M
;
	.module fdc765

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

	.globl  map_proc_always
	.globl	map_kernel

	.globl _fd765_do_nudge_tc
	.globl _fd765_do_recalibrate
	.globl _fd765_do_seek
	.globl _fd765_do_read
	.globl _fd765_do_write
	.globl _fd765_do_read_id
	.globl _fd765_motor_on
	.globl _fd765_motor_off
	.globl _fd765_intwait
	.globl _fd765_send_cmd
	.globl _fd765_send_cmd3

	.globl _fd765_track
	.globl _fd765_head
	.globl _fd765_sector
	.globl _fd765_status
	.globl _fd765_buffer
	.globl _fd765_is_user
	.globl _fd765_sectors
	.globl _fd765_drive

	.globl _diskmotor

	.area _COMMONMEM

FD_ST	.equ	0
FD_DT	.equ	1

;
; Twiddle the Terminal Count line to the FDC.
;
_fd765_do_nudge_tc:
	ld a,#0x05
	out (0xF8),a
	; Do we need a delay ?
	inc a
	out (0xF8),a
	ret

; Writes HL to the FDC register and moves on
fd765_tx_hl:
	ld a,(hl)
	inc hl
; Writes A to the FDC data register.

fd765_tx:
	ld c,a
fd765_tx_loop:
	in a, (FD_ST)
	add a
	jr nc, fd765_tx_loop
	add a
	ret c				; controller doesn't want it ?
	ld a,c
	out (FD_DT), a			; check if need delays
	ex (sp), hl			; controller time
	ex (sp), hl
	ret

fd765_sense:
	ld a,#8		; SENSE INTERRUPT STATUS
	call fd765_tx
	ret c
; Reads bytes from the FDC data register until the FDC tells us to stop (by
; lowering DIO in the status register).

fd765_read_status:
	ld hl, #_fd765_status
	ld c, #FD_DT
read_status_loop:
	in a, (FD_ST)
	add a,a
	jr nc, read_status_loop 	; ...low, keep waiting 
	add a,a
	ret nc				; ...low, no more data
	ini
	ex (sp),hl			; 12us for the controller
	ex (sp),hl			; to sort out the status bit
	ex (sp),hl
	ex (sp),hl
	jr read_status_loop		; next byte
_fd765_status:
	.ds 8				; 8 bytes of status data

; Sends the head/drive byte of a command.

send_head:
	ld hl, (_fd765_head)		; l = head h = drive)
	ld a, l
	add a
	add a
	add h
	jr fd765_tx

; Performs a RECALIBRATE command.

_fd765_do_recalibrate:
	ld a, #0x07				; RECALIBRATE
	call fd765_tx
	ld a, (_fd765_drive)			; drive #
	call fd765_tx
	jr wait_for_seek_ending

; Performs a SEEK command.

_fd765_do_seek:
	ld a, #0x0f				; SEEK
	call fd765_tx
	call send_head				; specified head, drive
	ld a, (_fd765_track)			; specified track
	call fd765_tx
	jr wait_for_seek_ending
_fd765_track:
	.db 0
_fd765_sector:
	.db 0
;
;	These two must remain adjacent see send_head
;
_fd765_head:
	.db 0
_fd765_drive:
	.db 0

; Waits for a SEEK or RECALIBRATE command to finish by polling SENSE INTERRUPT STATUS.
wait_for_seek_ending:
	in	a,(0xF8)
	and	#0x20			; FDC interrupt ?
	jr	z, wait_for_seek_ending

	call	fd765_sense
	ld	a,(_fd765_status)
	bit	5,a
	jr	z, wait_for_seek_ending

	; Now settle the head
	ld a, #30		; 30ms
wait_ms:
	push bc
wait_ms_loop:
	ld b,#0xDC
wait_ms_loop2:
	dec b
	jr nz, wait_ms_loop2
	dec a
	jr nz, wait_ms_loop
	pop bc
	ret

_fd765_motor_off:
	xor a
	ld (_diskmotor),a
	ld a,#0x0A
	out (0xF8),a
	ret

_fd765_motor_on:
	ld a,(_diskmotor)
	or a
	ret nz
	ld a,#0x09
	out (0xF8),a
	ld (_diskmotor),a
	; Now wait for spin up - should we ask the fdc instead ?
	ld e,#10		; 1 second (ouch)
wait2:
	; The classic Z80 KHz timing loop
	ld bc,#4000	; 4MHz
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
	call setup_read_or_write
	ld a, (_fd765_is_user)
	or a
	push af
	call nz, map_proc_always

	di				; performance critical,
					; run with interrupts off

	xor a
	call fd765_tx			; send the final unused byte
					; to fire off the command
	ld hl, (_fd765_buffer)
	ld a,(_fd765_sectors)
	ld e,a
	ld bc,#FD_DT

read_loop:
	in a,(FD_ST)
	add a
	jr nc, read_loop
	add a
	jp p, read_finished
	ini
	jr nz, read_loop
	dec e
	jr nz, read_loop

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
	call nz, map_proc_always

	di

	xor a
	call fd765_tx			; send the final unused 0 byte
					; to fire off the command
	ld hl, (_fd765_buffer)
	ld bc, #FD_DT
	ld a,(_fd765_sectors)
	ld e,a
write_loop:
	in a,(FD_ST)
	add a
	jr nc, write_loop
	add a
	jp p, write_finished
	outi
	jr nz, write_loop
	dec e
	jr nz, write_loop
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

_fd765_send_cmd:
	ld a,(hl)
	call fd765_tx
	jr c, cmdfail
	call send_head		; FIXME what should we be sending
cmdandout:
	ld hl,#0

	in a,(FD_ST)
	bit 5,a
	jr z, is_rdy
	
	ld a, #0x08		; SENSE INTERRUPT STATUS
	call fd765_tx
	ret c			; Failed to accept
	call fd765_read_status
	ld a,(_fd765_status)
	rla
	ret c			; Error
	rla
	ret c			; Abnormal
	; Nap to recover
	ld a,#30
	call wait_ms

is_rdy:
	call fd765_read_status
	ld hl,#_fd765_status
	ret
cmdfail:
	ld hl,#0
	ret

; Used for setup
_fd765_send_cmd3:
	call fd765_tx_hl
	call fd765_tx_hl
	call fd765_tx_hl
	jr cmdandout

_fd765_intwait:
	ld hl,#0xFFFF
	in a,(0xF8)
	bit 5,a
	ret z
	ld a,#0x08
	call fd765_tx
	call send_head
	call fd765_read_status
	ld a,(hl)
	bit 7,a
	jr nz, _fd765_intwait
	bit 6,a
	ret nz
	; Do we need a delay here ?
	ld b, #0x14			; give the controller time
fd765_wait:
	ld a, #0xB3
fd765_wait_1:
	ex (sp), hl
	ex (sp), hl
	dec a
	jr nz, fd765_wait_1
	djnz fd765_wait
	ret
