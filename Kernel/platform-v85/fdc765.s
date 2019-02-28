#
!
!		765 Floppy Controller Support
!
!	This is based upon the Amstrad NC200 driver by David Given
!
!	TODO
!	Initialize drive step rate etc (we rely on the firmware for now)
!	Step rate
!	Head load/unload times
!	Write off time
!

#include "../kernel-8080.def"

	.define _fd765_do_nudge_tc
	.define _fd765_do_recalibrate
	.define _fd765_do_seek
	.define _fd765_do_read
	.define _fd765_do_write
	.define _fd765_do_read_id
	.define _fd765_motor_on
	.define _fd765_motor_off

	.define _fd765_track
	.define _fd765_head
	.define _fd765_sector
	.define _fd765_status
	.define _fd765_buffer
	.define _fd765_is_user
	.define _fd765_sectors
	.define _fd765_drive

	.define diskmotor


.sect .common

!
! Twiddle the Terminal Count line to the FDC
!
_fd765_do_nudge_tc:
	mvi a,1
	out 0x1A
	dcr a
	out 0x1A
	ret

! Writes A to the FDC data register.

fd765_tx:
	push psw
fd765_tx_loop:
	in 19
	add a
	jnc fd765_tx_loop
	add a
	jc fd765_tx_exit		! controller doesn't want data ??
	pop psw
	out 18
	xthl			! check delay length is ok
	xthl
fd765_tx_exit:
	ret

! Reads bytes from the FDC data register until the FDC tells us to stop (by
! lowering DIO in the status register).

fd765_read_status:
	lxi h,_fd765_status
read_status_loop:
	in 0x19
	add a 				! RQM...
	jnc read_status_loop	 	! ...low, keep waiting 
	add a				! DIO...
	rnc				! ...low, no more data
	in 0x18
	mov m,a
	inx h
	xthl				! wait for the 765A
	xthl
	xthl
	xthl
	jmp read_status_loop		! next byte
_fd765_status:
	.data4 0			! 8 bytes of status data
	.data4 0

! Sends the head/drive byte of a command.

send_head:
	lhld _fd765_head		! l = head h = drive)
	mov a, l
	add a
	add a
	add h
	jmp fd765_tx

! Performs a RECALIBRATE command.

_fd765_do_recalibrate:
	mvi a,0x07				! RECALIBRATE
	call fd765_tx
	lda _fd765_drive			! drive
	call fd765_tx
	jmp wait_for_seek_ending

! Performs a SEEK command.

_fd765_do_seek:
	mvi a,0x0f				! SEEK
	call fd765_tx
	call send_head				! specified head, drive #0
	lda _fd765_track			! specified track
	call fd765_tx
	jmp wait_for_seek_ending
_fd765_track:
	.data1 0
_fd765_sector:
	.data1 0
!
!	These two must remain adjacent see send_head
!
_fd765_head:
	.data1 0
_fd765_drive:
	.data1 0

! Waits for a SEEK or RECALIBRATE command to finish by polling SENSE INTERRUPT STATUS.
wait_for_seek_ending:
	mvi a,0x08				! SENSE INTERRUPT STATUS
	call fd765_tx
	call fd765_read_status

	lda _fd765_status
	ani 0x20
	jz wait_for_seek_ending

	! Now settle the head
	mvi a,60		! 30ms @ 6MHz

wait_ms:
	push b
wait_ms_loop:
	mvi b,0xDC
wait_ms_loop2:
	dcr b			! 4
	jnz wait_ms_loop2	! 10 if taken
	dcr a			! 4
	jnz wait_ms_loop	! 10 if taken
	pop b
	ret

_fd765_motor_off:
	xra a
	out 0x1B
	sta diskmotor
	ret

_fd765_motor_on:
	lda diskmotor
	ora a
	rnz
	! Take effect
	sta diskmotor
	! Now wait for spin up

	mvi e,10
wait2:
	lxi b,0x61A8		! 6MHz
wait1:
	dcx b			!	6
	mov a,b			!	4
	ora c			!	4
	jnz wait1		!	10 (taken)	24 per loop
	dcr e			!	4
	jnz wait2		!	10 (taken)
	ret
!
! Reads a 512-byte sector, after having previously saught to the right track.
!
! We need to be doubly careful here as the 765A has a 'feature' whereby it
! won't report an overrun on the last byte so we must always make timing
!
_fd765_do_read:
	mvi a,0x46			! READ SECTOR MFM

	call setup_read_or_write

	lda _fd765_is_user
	ora a
	push psw
	cnz map_process_always

	di				! performance critical,
					! run with interrupts off

	xra a
	call fd765_tx			! send the final unused byte
					! to fire off the command	
	lhld _fd765_buffer
	mvi e,0
	jmp read_wait
!
!	First 256 bytes. Fix me - timings/optimization eg can we avoid
!	second add a on an 8080
!
read_loop:
	in 0x18
	mov m,a
	inx h
	dcr e
	jz read_wait2
read_wait:
	in 0x19			! read the fdc status
	add a			! bit 7 into C
	jnc read_wait		! 7 clear => not ready
	add a			! bit 6 into C bit 5 into sign
	jp read_loop		! 5 set => still transferring
	jmp read_finished
!
!	Second 256 bytes
!
read_loop2:
	in 0x18
	mov m,a
	inx h
	dcr e
	jz read_finished
read_wait2:
	in 0x19
	add a
	jnc read_wait2
	add a
	jp read_loop2
	jmp read_finished
!
!	And done
!
read_finished:
	shld _fd765_buffer
	! This is wrong - we should issue it before we read the last
	! according to some docs ?
	call _fd765_do_nudge_tc		! Tell FDC we've finished
	ei

	call fd765_read_status

	pop psw
	rz
	jmp map_kernel

!
!	Write is much like read just the other direction
!
_fd765_do_write:
					! interrupts off
	mvi a,0x45			! WRITE SECTOR MFM
	call setup_read_or_write

	lda _fd765_is_user
	ora a
	push psw
	cnz map_process_always

	di

	xra a
	call fd765_tx			! send the final unused 0 byte
					! to fire off the command	
	lhld _fd765_buffer
	mvi e,0
	jmp write_wait

write_loop:
	mov a,m
	out 0x18
	inx h
	dcr e
	jz write_wait2
write_wait:
	in 0x19
	add a
	jnc write_wait
	add a
	jp write_loop
	jmp write_finished
write_loop2:
	mov a,m
	out 0x18
	inx h
	dcr e
	jz write_finished
write_wait2:
	in 0x19
	add a
	jnc write_wait2
	add a
	jp write_loop2
write_finished:
!	shld _fdc765_buffer
	call _fd765_do_nudge_tc		! Tell FDC we've finished
	ei
	call fd765_read_status
	pop psw
	rz
	jmp map_kernel

! Given an FDC opcode in A, sets up a read or write.

setup_read_or_write:
	call fd765_tx			! 0: send opcode (in A)
	call send_head			! 1: specified head, drive #0
	lda _fd765_track		! 2: specified track
	call fd765_tx
	lda _fd765_head			! 3: specified head
	call fd765_tx
	lda _fd765_sector		! 4: specified sector
	mov b, a
	call fd765_tx
	mvi a,2				! 5: bytes per sector: 512
	call fd765_tx
	lda _fd765_sectors
	add b				! add first sector
	dcr a				! 6: last sector (*inclusive*)
	call fd765_tx
	mvi a,0x2A   			! 7: Gap 3 length (5.25 and 3)
					! for 3.5 use 1B
	call fd765_tx
	! We return with the final unused 0 value not written. We need all
	! the other stuff lined up before we write this.
	ret

_fd765_buffer:
	.data2 0
_fd765_is_user:
	.data1 0
_fd765_sectors:
	.data1 0
diskmotor:
	.data1 0

! Read the next sector ID off the disk.
! (Only used for debugging.)

_fd765_do_read_id:
	mvi a,0x4a 				! READ MFM ID
	call fd765_tx
	call send_head				! specified head, drive 0
	jmp fd765_read_status
