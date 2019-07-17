		.area BOOT (ABS)

		.org 0xF000

FD_ST		.equ	0
FD_DT		.equ	1
stack		.equ	0xF400
fd765_statbuf	.equ	0xF402
start		.equ	0x0088		; start of execution once loaded

;
;	fd765_cyl must not move without updating the uzi startup code for
;	this platform
;
fd765_lead:	.db 0, 0, 40, 9, 2, 1, 0x3, 2
		.db 0, 0, 0, 0, 0, 0, 0, 0
;
;	The boot rom calls us right here
;
fd765_start:	jp fd_go

fontnum:	.db 0x38, 0x6c, 0xc6, 0xd6, 0xc6, 0x6c, 0x38, 0x00
		.db 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00
		.db 0x7c, 0xc6, 0x06, 0x1c, 0x30, 0x66, 0xfe, 0x00
		.db 0x7c, 0xc6, 0x06, 0x3c, 0x06, 0xc6, 0x7c, 0x00
		.db 0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00
		.db 0xfe, 0xc0, 0xc0, 0xfc, 0x06, 0xc6, 0x7c, 0x00
		.db 0x38, 0x60, 0xc0, 0xfc, 0xc6, 0xc6, 0x7c, 0x00
		.db 0xfe, 0xc6, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00
		.db 0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0x00
		.db 0x7c, 0xc6, 0xc6, 0x7e, 0x06, 0x0c, 0x78, 0x00
; fortunately error is only 3 symbols long
fonte:		.db 0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00
fontr:		.db 0x00, 0x00, 0xdc, 0x76, 0x60, 0x60, 0xf0, 0x00
fonto:		.db 0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00

digiprint:
		ld h, #0xF0		; font high
		add a, a
		add a, a
		add a, a
		add #19	;(fontnum % 256)	; and move to align
		ld l, a
ldir8:
fd765_sensep:	ld bc, #8		; borrow the 8 to save a byte
		ldir
		ret

		
; bc = length, port
; hl = command buffer
fd765_sendcmd:
		in a, (FD_ST)
		add a, a
		jr nc, fd765_sendcmd
		jp m, fd765_intdatain		; wtf ???
		outi				; transfer command byte
		ex (sp), hl
		ex (sp), hl
		ex (sp), hl
		ex (sp), hl
		jr nz, fd765_sendcmd
fd765_intdatain:
		ret				; need to indicate an error
						; somehow on data in error
fd765_status:
		ld hl, #fd765_statbuf
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
;	We rely on this exiting with C = FD_DT
;
fd765_intwait:
		ld bc, #(1 * 256 + FD_DT)	; send SENSE INTERRUPT STATUS
		in a, (0xF8)
		bit 5, a
		ret z				; wait for the 765 int to go off
		ld hl, #fd765_sensep + 1	; a suitable 0x08 in the code
		call fd765_sendcmd
		call fd765_status
		ld a, (fd765_statbuf)
		bit 7, a			; error
		jr nz, fd765_intwait
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
		
		
;		C points at FD_DT
fd765_xfer_in:
		in a, (FD_ST)
		add a
		jr nc, fd765_xfer_in		; bit 7 clear
		add a
		jp p, fd765_boot_error		; bit 5 clear (short data ???)
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

fd_go:
		di
		ld sp, #stack			; so we can bank flip
		ld a, #0x84			; map the screen into bank 4/5
		out (0xf1), a			; and put them at 0x4000/0x8000
		inc a
		out (0xf2), a
		ld hl, #0x4000 + 0x5A00		; base for roller ram
		ld de, #0x8000			; bank 4 offset 0
		ld b, #32
roller_fill:	push bc
		ld b, #8
		ld c, e
roller_8:
		ld (hl), c			; fill the low 3 bits 0-7
		inc hl
		ld (hl), d			; rest is bank and offset
		inc hl
		inc c
		djnz roller_8			; do a character line
		push hl
		ld hl, #360			; avoids the shift
		add hl, de			; Move on 720 bytes
		ex de, hl			; back into de
		pop hl
		pop bc				; do the scan lines
		djnz roller_fill		; Next roller char
		ld hl, #0x4000			; patterned screen
		ld de, #0x2D0			; first set of 8 lines
whitetop:
		ld (hl), #0xff
		inc hl
		dec de
		ld a, d
		or e
		jr nz, whitetop
		ld de, #0x5A00 / 2 - 0x2D0	; pairs of bytes
greyscreen:
		ld (hl), #0xAA			; layout makes this a doddle
		inc hl
		ld (hl), #0x55
		inc hl
		dec de
		ld a, d
		or e
		jr nz, greyscreen
		ld a, #0x81
		out (0xf1), a			; put memory back
		inc a
		out (0xf2), a
		ld a, #0xAD			; bank 5 offset D
		out (0xF5), a
		xor a
		out (0xF6), a
		ld a, #0x07
		out (0xF8), a
		ld a, #0x40
		out (0xF7), a

;
;	9 * 512 byte sectors/track on the boot image. We load 12 tracks
;
fd765_boot:
		ld a, #0x84		; map display
		out (0xf2), a
		ld a, (fd765_cyl)
		ld de, #0x8000 + 5760 + 352
		call digiprint
		ld a, (fd765_sector)
		call digiprint
		ld a, #0x82
		out (0xf2), a		; back to main memory
		call fd765_begin
		jr nz, fd765_boot_error
;
;		Sector loaded ok, we use the first 0x80 bytes of the loaded
;		code to do the progress bar
;
		ld h, #0
		ld a, #0x84
		out (0xf2), a		; map in the screen
		rst 0
;		nop
		ld a, #0x82
		out (0xf2), a		; map back the loader
		ld a, (fd765_sector)
		inc a
		cp #10
		jr nc, fd765_nextcyl
		ld (fd765_sector), a
		ld (fd765_seclast), a
		jr fd765_boot
fd765_nextcyl:
		ld a, #1
		ld (fd765_sector), a
		ld (fd765_seclast), a
		ld a, (fd765_cyl)
		inc a
		cp #13
		jr nc, fd765_done
		ld (fd765_cyl), a
		ld (fd765_cyl2), a
		jr fd765_boot

; remap display, print Error message
fd765_boot_error:
		ld a, #0x84
		out (0xf2), a
		ld hl, #fonte
		ld de, #0x8000 + 5760 + 344 + 720
		call ldir8
		ld hl, #fontr
		push hl
		call ldir8
		pop hl
		push hl
		call ldir8
		ld hl, #fonto
		call ldir8
		pop hl
		call ldir8
		halt
		

fd765_done:	jp start			; go go go....

fd765_begin:
;		The ROM did this bit for us!
;		call fd765_intwait
;		ld hl, fd765_setup		; set up drive parameters
;		ld bc, #(5 * 256 + FD_DT)	; 5 byte command
;		call fd765_sendcmd		; set up the drive
;		call fd765_status

		ld a, #6			; clear terminal count flag
		out (0xF8), a

		call fd765_intwait
		ld hl, #fd765_seek
		ld b, #3			; C was set by intwait
		call fd765_sendcmd
		call fd765_intwait
		ld b, #9			; READ, C was set by intwait
		ld hl, #fd765_read_data
		call fd765_sendcmd		; send the READ DATA command
		ld de, (codebase)
		ld hl, #512
		add hl, de
		ld (codebase), hl		; move on 512
		ex de, hl
		ld e, #2			; 512 bytes
		ld b, #0
		call fd765_xfer_in		; copy the data
		call fd765_status		; get the status bits
		; failed ?
		ld a, (fd765_statbuf)
		and #0xCB
		ret

codebase:	.dw 0x0000

fd765_read_data:
		.db 0x66		; Normal MFM read
		.db 0		; Drive 0, head 0
fd765_cyl:	.db 0		; cylinder
		.db 0		; head
fd765_sector:	.db 2		; sector
		.db 2		; 512 bytes
fd765_seclast:	.db 2		; last sector
		.db 0x2A		; gap length
		.db 0xFF		; no room for this byte so we overwrite the
				; jp further down 8)
fd765_sense:	.db 0x08		; Sense interrupt
fd765_seek:	.db 0x0F
		.db 0
fd765_cyl2:	.db 0
		; Checksum
checksum:	.db 0x3d
