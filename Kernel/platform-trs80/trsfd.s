
FDCSTAT		.equ	0xF0
FDCA		.equ	0xF0
FDCTRK		.equ	0xF1
FDCSEC		.equ	0xF2
FDCDAT		.equ	0xF3
DRVSEL		.equ	0xF4

;
;	Write a command to the FDC then nap briefly while the
;	FDC digests it
;
writecmd:
		out (FDCA), a		; off we go
cmd_1:					; wait for controller
		ld b, 18
		djnz cmd_1
		ret


;
;	Set the desired track
;
;	B = the track we think we are on
;	C = the sector we will want
;	D = the track we will want
;	E = the step rate (0-3) 
;
seekto:		call reselect
		out (FDCTRK), b		; current track
		out (FDCSEC), c		; sector we want
		out (FDCDATA), d	; track we want
		ld a, b
		cp d			; do we think we are there ?
		ld b, 0x18		; SEEK
		jr z, seekto_1
		ld b, 0x1C		; SEEK with verify
seekto_1:	ld a, b
		or e			; step rate
		call writecmd
		ret

;
;	Perform a read transfer once we have been through the selection
;	process
;
;	d = track
;	
;
;
read_xfer:	ld c, FDCTRK
		out (c), d		; track we want
		
		call writecmd

		ld bc, FDCDAT		; 256 bytes/sector, and load c with
					; our port
		ld e, 0x16		; mask of bits we are checking
;		 
;	Wait for DRQ, and then block transfer the bytes
;
wait_drq:	in a, (FDCSTAT)
		and e
		jr z, wait_drq
		ini
		di
		ld a, d
wait_go:	out (DRVSEL), a		; wait stating
		ini			; byte in
		jr nz, wait_go		; repeat
;
;	Sector data has landed
;
		call fdcwait
;	status in A
		ret
		

fdcwait:
		in a, (FDCSTAT)
		bit 0, a		; need a in return so don't use the rra
					; shortcut
		ret z			; not busy ???
		ld a, drvsel
		out (DRVSEL), a
		jr fdcwait


select:		push bc
		call reselect
		ld b, a			; save the status
		rlca			
		rla			; magic - move bits 6/4 into bits 7/4
		sraa			
		and 0x90
		ld c, a
		bit 7, a		; double density ?
		jr z, nocomp
		ld a, precomp_start
		cp d			; track needs precomp ?
		jr nc, nocomp
		set 5, c
		ld a, drivesel
		and 0x0f
		or c

		out (DRVSEL), a		; select drive
		out (DRVSEL), a		; in case the trash80 wasn't listening
		
		bit 1, b		; delay time ?
		call z, fdcspin		; long
		call fdcspin		; short (1/2 long)
		pop bc
		ret

fdcspin:	ld b, 0x7f
		pause...
		ret



fdc_

