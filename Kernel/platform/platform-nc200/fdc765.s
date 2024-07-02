;
;		765 Floppy Controller Support
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

	.globl _fd765_track
	.globl _fd765_head
	.globl _fd765_sector
	.globl _fd765_status
	.globl _fd765_buffer
	.globl _fd765_is_user
	.globl _fd765_sectors

	.area _COMMONMEM

	FD_ST	.equ	0xe0
	FD_DT	.equ	0xe1

; The Ranger ROM in the NC200 does mysterious things with port 0x20.
;   bit 0: TC (documented)
;   bit 1: always set
;   bit 2: set on motor power on

; Twiddle the Terminal Count line to the FDC.

_fd765_do_nudge_tc:
	ld a, #0x83
	out (0x20), a
	dec a
	out (0x20), a
	ret

; Writes A to the FDC data register.

fd765_tx:
	ex af, af'
fd765_tx_loop:
	in a, (FD_ST)
	rla							; RQM...
	jr nc, fd765_tx_loop		; ...low, keep waiting

	ex af, af'
	out (FD_DT), a
	ret

; Reads bytes from the FDC data register until the FDC tells us to stop (by
; lowering DIO in the status register).

fd765_read_status:
	ld hl, #_fd765_status
	ld c, #FD_DT
read_status_loop:
	in a, (FD_ST)
	rla 						; RQM...
	jr nc, read_status_loop 	; ...low, keep waiting 
	rla							; DIO...
	ret nc						; ...low, no more data
	ini							; (hl)++ = port(c); b--
	jr read_status_loop
_fd765_status:
	.ds 8						; 8 bytes of status data

; Sends the head/drive byte of a command.
; (Drive #0 is always used.)

send_head:
	ld a, (_fd765_head)
	add a
	add a
	jr fd765_tx

; Performs a RECALIBRATE command.

_fd765_do_recalibrate:
	ld a, #0x07					; RECALIBRATE
	call fd765_tx
	ld a, #0x00 				; drive #0
	call fd765_tx
	jr wait_for_seek_ending

; Performs a SEEK command.

_fd765_do_seek:
	ld a, #0x0f					; SEEK
	call fd765_tx
	call send_head				; specified head, drive #0
	ld a, (_fd765_track)		; specified track
	call fd765_tx
	jr wait_for_seek_ending
_fd765_track:
	.db 0
_fd765_head:
	.db 0
_fd765_sector:
	.db 0

; Waits for a SEEK or RECALIBRATE command to finish by polling SENSE INTERRUPT STATUS.
wait_for_seek_ending:
	ld a, #0x08					; SENSE INTERRUPT STATUS
	call fd765_tx
	call fd765_read_status

	ld a, (_fd765_status)
	bit 5, a					; SE, seek end
	jr z, wait_for_seek_ending
	ret

; Reads a 512-byte sector, after having previously saught to the right track.

_fd765_do_read:
	di							; performance critical, run with interrupts off
	ld a, #0x46					; READ SECTOR MFM
	call setup_read_or_write

	ld a, (_fd765_is_user)
	or a
	call nz, map_proc_always
	
	xor a						; 8: Data length (unused)
	call fd765_tx

	ld hl, (_fd765_buffer)
	ld c, #FD_DT
	ld b, #0
	ld a, (_fd765_sectors)
	add a
	ld e, a
read_loop:
	in a, (FD_ST)
	rla							; RQM...
	jr nc, read_loop      		; ...low, keep waiting
	rla							; DIO (ignore)
	rla							; EXM...
	jr nc, read_finished		; ...low, transfer complete
	ini							; (HL)++ = port(C), B--
	jr nz, read_loop			; inner loop: 256 iterations
	dec e
	jr nz, read_loop			; outer loop: 2 iterations
read_finished:
	ld (_fd765_buffer), hl
	call _fd765_do_nudge_tc		; Tell FDC we've finished
	call fd765_read_status
	ei

	ld a, (_fd765_is_user)
	or a
	call nz, map_kernel
	ret

_fd765_do_write:
	di							; performance critical, run with interrupts off
	ld a, #0x45					; WRITE SECTOR MFM
	call setup_read_or_write

	ld a, (_fd765_is_user)
	or a
	call nz, map_proc_always
	
	xor a						; 8: Data length (unused)
	call fd765_tx

	ld hl, (_fd765_buffer)
	ld c, #FD_DT
	ld b, #0
	ld a, (_fd765_sectors)
	add a
	ld e, a
write_loop:
	in a, (FD_ST)
	rla							; RQM...
	jr nc, write_loop      		; ...low, keep waiting
	rla							; DIO (ignore)
	rla							; EXM...
	jr nc, write_finished		; ...low, transfer complete
	outi						; port(C) = (HL)++, B--
	jr nz, write_loop			; inner loop: 256 iterations
	dec e
	jr nz, write_loop			; outer loop: 2 iterations
write_finished:
	ld (_fd765_buffer), hl
	call _fd765_do_nudge_tc		; Tell FDC we've finished
	call fd765_read_status
	ei

	ld a, (_fd765_is_user)
	or a
	call nz, map_kernel
	ret

; Given an FDC opcode in A, sets up a read or write.

setup_read_or_write:
	call fd765_tx				; 0: send opcode (in A)
	call send_head				; 1: specified head, drive #0
	ld a, (_fd765_track)		; 2: specified track
	call fd765_tx
	ld a, (_fd765_head)			; 3: specified head
	call fd765_tx
	ld a, (_fd765_sector)		; 4: specified sector
	ld b, a
	call fd765_tx
	ld a, #2					; 5: bytes per sector: 512
	call fd765_tx
	ld a, (_fd765_sectors)		
	add b						; add first sector
	dec a						; 6: last sector (*inclusive*)
	call fd765_tx
	ld a, #27  					; 7: Gap 3 length (27 is standard for 3.5" drives)
	call fd765_tx
	;	Don't send final 0 byte - caller will do that to avoid
	;	timing issues
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
