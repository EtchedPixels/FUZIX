;
;		765 Floppy Controller Support
;
;	This is based upon the Amstrad NC200 driver by David Given and 
;  this example https://cpctech.cpcwiki.de/source/fdcload.html from Kevin Thacker site
;
;	It differs on the CPC in the following ways
;
;	- The timings are tighter so we use in a,(c) jp p and other
;	  tricks to make the clocks. Even so it should be in uncontended RAM
;
;	- The CPC doesn't expose the tc line, so if the 765 decides to
;	  expect more data or feed us more data all we can do is dump it or
;	  feed it crap until it shuts up
;
;	- We do motor and head loading delays (possibly some of those should
;	  be backported - FIXME)
;
;	- We don't hang if the controller tells us no more data when we
;	  think we need to feed it command bytes (BACKPORT NEEDED)
;
;	TODO
;	Initialize drive step rate etc (we rely on the firmware for now)
;	Step rate
;	Head load/unload times
;	Write off time	af
;	(12ms step 30ms head stabilize, 4ms head load, max (0xf) head
;	unload)
;
		.module fdc765

		.include "kernel.def"
		.include "../../cpu-z80/kernel-z80.def"


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
	.globl _fd765_map
	.globl _fd765_sectors
	.globl _fd765_drive

	.globl _vtborder


	.globl diskmotor

	.area _COMMONMEM

diskmotor:
	 .db 0
;
; Twiddle the Terminal Count line to the FDC. Not supported by the
; CPC
;
_fd765_do_nudge_tc:
	ret

; Writes A to the FDC data register.

fd765_tx:
	push bc
	ld bc,#0xfb7e					;; I/O address for FDC main status register
	push af						;;
	fwc1: in a,(c)				;; 
	add a,a						;; 
	jr nc,fwc1					;; 
	add a,a						;; 
	jr nc,fwc2					;; 
	pop af						;; 
	ret							

	fwc2: 
	pop af				;; 

	inc c						;; 
	out (c),a					;; write command byte 
	dec c						;; 

	;; some FDC documents say there must be a delay between each
	;; command byte, but in practice it seems this isn't needed on CPC.
	;; Here for compatiblity.
	ld a,#5				;;
	fwc3: dec a			;; 
	jr nz,fwc3			;; 
	pop bc
	; FIXME: is our delay quite long enough for spec ?
	; might need them to be ex (sp),ix ?
	ret

; Reads bytes from the FDC data register until the FDC tells us to stop (by
; lowering DIO in the status register).

fd765_read_status:
	ld hl, #_fd765_status
	ld bc,#0xfb7e
	fr1:
	in a,(c)
	cp #0xc0 
	jr c,fr1
	
	inc c 
	in a,(c) 
	dec c 
	ld (hl),a 
	inc hl 

	ld a,#5 
	fr2: 
	dec a 
	jr nz,fr2
	in a,(c) 
	and #0x10 
	jr nz,fr1


	ret
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
	call send_head				; specified head, drive #0
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

	ld a, #0x08				; SENSE INTERRUPT STATUS
	call fd765_tx
	call fd765_read_status

	ld a, (#_fd765_status)
	bit 5, a				; SE, seek end
	jr z, wait_for_seek_ending

	bit 4,a
	
	ret

	; Now settle the head (FIXME: what is the right value ?)
	ld a, #30		; 30ms
;
;	This assumes uncontended timing
;
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
	push bc
	ld bc,#0xfa7e
	xor a
	ld (diskmotor),a
	out (c),a
	pop bc
	ret

_fd765_motor_on:
	ld a,(diskmotor)
	or a
	ret nz
	ld a,#0x01
	ld (diskmotor),a
	; Take effect
	ld bc,#0xfa7e
	out (c),a
	; Now wait for spin up

	ld e,#10		; FIXME right value ?? 	
wait2:
	; The classic Z80 KHz timing loop
	ld bc,#3548	; 3.548MHz for spectrum, should change for cpc.FIXME
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

	ld hl, (_fd765_buffer)
	ld a, (_fd765_map)
	or a
	;push af
	jr z, cont_read_nomap
	ld bc,#0x7fff
	out (c),a
cont_read_nomap:
	ld bc,#0x7f10
	out (c),c
	ld c,#0x46		;Cyan
	out (c),c

	di				; performance critical,
					; run with interrupts off
	xor a
	call fd765_tx			; send the final unused byte
					; to fire off the command	
	ld bc, #0xfb7e

fdc_data_read: 
	in a,(c)				          ;; FDC has data and the direction is from FDC to CPU
	jp p,fdc_data_read		;; 
	and #0x20					;; "Execution phase" i.e. indicates reading of sector data
	jp z,fdc_read_end 		

	inc c					;; BC = I/O address for FDC data register
	in a,(c)				;; read from FDC data register
	ld (hl),a				;; write to memory
	dec c					;; BC = I/O address for FDC main status register
	inc hl					;; increment memory pointer
	jp fdc_data_read

fdc_read_end:
	
	ld bc,#0x7fc2
	out (c),c
	ld (_fd765_buffer), hl
	call _fd765_do_nudge_tc		; Tell FDC we've finished
	ei

	call fd765_read_status
	call tc_fix

	ld bc,#0x7f10
	out (c),c
	ld a,(_vtborder)
	out (c),a
	ret
;
;	We will get an error reported that the command did not complete
;	because the tc bit is not controllable. Spot that specific error
;	and ignore it.
;
tc_fix:
	ld hl,#_fd765_status
	ld a,(hl)
	and #0xC0
	cp #0x40
	ret nz
	inc hl
	bit 7,(hl)
	ret z
	res 7,(hl)
	dec hl
	res 6,(hl)
	ret

;
;	Write is much like read just the other direction
;
_fd765_do_write:
					; interrupts off
	ld a, #0x45			; WRITE SECTOR MFM
	call setup_read_or_write

	ld hl, (_fd765_buffer)
	ld a, (_fd765_map)
	or a
	;push af
	jr z, cont_read_nomap
	ld bc,#0x7fff
	out (c),a
cont_write_nomap:

	ld bc,#0x7f10
	out (c),c
	ld c,#0x47		;Pink
	out (c),c

	di

	xor a
	call fd765_tx			; send the final unused 0 byte
					; to fire off the command	
	ld bc, #0xfb7e
fdc_data_write: 
	in a,(c)				          ;; FDC has data and the direction is from FDC to CPU
	jp p,fdc_data_write		;; 
	and #0x20					;; "Execution phase" i.e. indicates reading of sector data
	jp z,fdc_write_end 		

	inc c					;; BC = I/O address for FDC data register
	ld a,(hl)				;; read from memory
	out (c),a				;; write to FDC data register
	dec c					;; BC = I/O address for FDC main status register
	inc hl					;; increment memory pointer
	jp fdc_data_write

fdc_write_end:

	ld bc,#0x7fc2
	out (c),c

	ld (_fd765_buffer), hl
	call _fd765_do_nudge_tc		; Tell FDC we've finished
	ei
	call fd765_read_status
	call tc_fix

	ld bc,#0x7f10
	out (c),c
	ld a,(_vtborder)
	out (c),a

	ret
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
_fd765_map:
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
