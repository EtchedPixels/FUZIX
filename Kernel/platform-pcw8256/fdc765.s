;
;		765 Floppy Controller Support
;
		.module fdc765

		.include "kernel.def"
		.include "../kernel.def"

		.globl	_fd765_read_sector
		.globl	_fd765_write_sector
		.globl  _fd765_cmd2
		.globl  _fd765_cmd3
		.globl  _fd765_intwait
		.globl  _fd765_buffer
		.globl  _fd765_user
		.globl  _fd765_rw_data
		.globl  _fd765_cmdbuf
		.globl  _fd765_statbuf

		.globl  map_process_always
		.globl	map_kernel
		.globl _int_disabled

		.area _COMMONMEM

		FD_ST	.equ	0
		FD_DT	.equ	1

;
;	Issue a command to the 765 floppy controller. On the Amstrad we
;	apparently have to twiddle our thumbs between bytes to keep the
;	controller happy
;
fd765_sendcmd:
		in a, (FD_ST)
		add a, a
		jr nc, fd765_sendcmd		; busy
		jp m, fd765_sendabort		; not expecting command ??
		outi				; byte out
		ex (sp), hl			; controller time
		ex (sp), hl
		ex (sp), hl
		ex (sp), hl
		jr nz, fd765_sendcmd		; next byte ?
		ret
;
;	Mid command send the controller said it was not expecting more
;	command bytes. This means we screwed up
;
fd765_sendabort:
		or a				; NZ
		; FIXME
		ret

;
;	Get the controller status int fd765_statbuf. Each command returns
;	a series of status bytes. As with commands we have to collect the
;	status data with pauses between bytes
;
fd765_status:
		ld hl, #_fd765_statbuf
fd765_status_1:
		in a, (FD_ST)
		add a, a
		jr nc, fd765_status_1
	        add a, a
		ret nc				; bit 6 was clear, so done
		ini
		ex (sp), hl			; waste time for the 765
		ex (sp), hl			; to recover
		ex (sp), hl
		ex (sp), hl
		jr fd765_status_1
;
;	The status bytes are all sent. Return with HL (and A) holding the
;	first status byte
;
fd765_status_a:
		call fd765_status
		ld h, #0
		ld a, (_fd765_statbuf)
		ld l, a
		ret

;
;	We rely on this exiting with C = FD_DT
;
;	General trick, we load B with the command length and C with the
;	FD_DT register in one go
;
_fd765_intwait:
		ld bc, #1 * 256 + FD_DT		; send SENSE INTERRUPT STATUS
		;
		;	Check what is pending on the interrupt side
		;
		ld hl, #0xffff			; report -1 if not pending
		in a, (0xF8)
		bit 5, a
		ret z				; wait for the 765 int to go off
		;
		;	Send the sense command, get the status back
		;
		ld hl, #fd765_sense		; sense command
		call fd765_sendcmd
		call fd765_status_a
		bit 7, a			; error
		jr nz, _fd765_intwait
		bit 6, a			; abnormal
		ret nz
		ld b, #0x14			; give the controller time
fd765_wait:	ld a, #0xB3
fd765_wait_1:	ex (sp), hl
		ex (sp), hl
		ex (sp), hl
		ex (sp), hl
		dec a
		jr nz, fd765_wait_1
		djnz fd765_wait
		ret

;
;	Block transfer bytes from the controller. We play some cute games
;	with flags in order to keep this loop tight. We do *not* have a lot
;	of time here because the bytes flow from the FDC to us at media head
;	speed and if we miss one we lose.
;
;		C points at FD_DT
fd765_xfer_in:
		in a, (FD_ST)
		add a
		jr nc, fd765_xfer_in		; bit 7 clear
fd765_xfer_in2:
		add a
		jp p, fd765_xfer_error		; bit 5 clear (short data ???)
		ini				; transfer a byte
		jr nz, fd765_xfer_in		; next byte
		dec e
		jr nz, fd765_xfer_in
;
;		Sector transferred
;
fd765_xfer_done:ld a, #5
		out (0xF8), a			; terminate flag set
		ret
fd765_xfer_error:
		or a				; NZ
		; FIXME
		ret
;
;	We enter the block transfer loop here. The first time around
;	the loop we spin until a byte is ready and then disable interrupts
;	before reading the bytes. We don't re-enable IRQs that is up to
;	our caller.
;
fd765_xfer_indi:
		in a, (FD_ST)
		add a
		jr nc, fd765_xfer_indi		; bit 7 clear
		di
		jr fd765_xfer_in2

;
;	Block transfer out. As with a block transfer from the controller we
;	have to keep up so must do the transfer with interrupts off. Other
;	that the direction this works the same as input. In fact if we were
;	sure this wasn't ever going to end up in ROM we could self modify
;
;		C points at FD_DT
fd765_xfer_out:
		in a, (FD_ST)
		add a
		jr nc, fd765_xfer_out		; bit 7 clear
fd765_xfer_out2:
		add a
		jp p, fd765_xfer_error		; bit 5 clear (short data ???)
		outi				; transfer a byte
		jr nz, fd765_xfer_in		; next byte
		dec e
		jr nz, fd765_xfer_in
;
;		Sector transferred
;
		ld a, #5
		out (0xF8), a			; terminate flag set
		ret

fd765_xfer_outdi:
		in a, (FD_ST)
		add a
		jr nc, fd765_xfer_outdi		; bit 7 clear
		di
		jr fd765_xfer_out2

;
;	Read a disc sector.
;
_fd765_read_sector:
		ld a,(_int_disabled)
		push af
		ld a, (_fd765_user)
		or a
		jr z, read_kern
;
;	FIXME: this isn't sufficient for swap because of the fact we
;	may be swapping over the 48K boundary mark
;
;	Plus this is wrong as we need to pass HL = p->page of the task!
;
		call map_process_always
read_kern:
		ld a, #6
		out (0xf8), a
		call _fd765_intwait		; wait for controller
		;
		;	Send the rw_data command. The command has been
		;	modified by the caller to be the right read/write
		;	for the right track/sector
		;
		ld b, #9
		ld hl, #_fd765_rw_data
		call fd765_sendcmd
		jr nz, read_failed
		ld hl, (_fd765_buffer);
		ld e, #2
		ld b, #0			; 512 bytes
		call fd765_xfer_in
		jr nz, read_failed
read_status:					; clean up is shared
		call fd765_status_a		; with write method
		ld hl, #0
		and #0xF8
		jr nz, read_failed
		; read ok
read_out:
		call map_kernel
		pop af
		or a
		ret nz
		ei
		ret
read_failed:
		dec hl
		jr read_out

;
;	Write a sector. Very similiar to reading a sector and could be
;	be self modifying to save space.
;
_fd765_write_sector:
		ld a,(_int_disabled)
		push af
		ld a, (_fd765_user)
		or a
		jr z, write_kern
;
;	FIXME: as per read.
;
		call map_process_always
write_kern:
		ld a, #6
		out (0xf8), a
		call _fd765_intwait		; wait for controller
		ld b, #9
		ld hl, #_fd765_rw_data
		call fd765_sendcmd
		jr nz, read_failed
		ld hl, (_fd765_buffer);
		ld e, #2
		ld b, #0			; 512 bytes
		call fd765_xfer_out
		jr nz, read_failed
		jr read_status

_fd765_cmd2:	call _fd765_intwait
		ld b, #2
fd765_cmdop:
		ld hl, #_fd765_cmdbuf
		call fd765_sendcmd
		jr nz, read_failed
		call _fd765_intwait
		call fd765_status_a
		ret

_fd765_cmd3:	call _fd765_intwait
		ld b, #3
		jr fd765_cmdop

;
;	Where to put this ? As we may flip between processes including high
;	pages it is not clear where this data belongs. For now keep it in
;	common. Swap + banked raises some architectural issues (where is the
;	stack going ????) so it may be better to address this in the core
;	swap code instead. For non swap cases common will do fine for now.
;

_fd765_buffer:	.dw 0		; Buffer pointer
_fd765_user:	.db 0		; 0 - kernel 1 - user

_fd765_rw_data: .db 0x66	; Normal MFM read
		.db 0		; Drive 0, head 0
		.db 0		; cylinder
		.db 0		; head
		.db 2		; sector
		.db 2		; 512 bytes
		.db 2		; last sector
		.db 0x2A	; gap length
		.db 0xFF	; unused

fd765_sense:	.db 0x08	; Sense interrupt

_fd765_cmdbuf:	.db 0x0F
		.db 0
		.db 0
		.db 0
_fd765_statbuf:
		.ds 8
