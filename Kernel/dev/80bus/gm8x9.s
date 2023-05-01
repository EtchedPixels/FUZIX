	.module gm8x9

	.include "kernel.def"
	.include "../kernel-z80.def"

	.globl _gm8x9_ioread
	.globl _gm8x9_iowrite
	.globl _gm8x9_reset
	.globl _gm8x9_seek
	.globl _gm8x9_restore
	.globl _gm8x9_restore_test

	.globl _io_page
	.globl _gm8x9_steprate
	.globl map_process_a
	.globl map_kernel

;
;	The 809 is a 1797 with 5.25" (optionally 8") disk support
;	and unlike the Henelec supports double density, and does automatic
;	hardware delays.
;
;	Usual formats are
;	SD: 35 track 18 sector 128 bytes/sector dual sided
;	DD: 35 track 10 sector 512 bytes/sector dual sided
;
;	The GM829 is similar but also has an optional SASI interface. The
;	MAP80 VFC is GM809 compatible on the floppy side.
;
;	E0-E3 are the 1797
;
;	E4 0-3 selects drives 0-3
;	E4 4 switches density
;	E4 5 switches drive type (829 only)
;
;	Side select is in type II commands (bit 3) and bit 3 of ctrl ?
;
;	Writing E4 also turns on the motor
;
;	On the read side it's the usual bit 7 is DRQ bit 0 is INTRQ
;	Ready *may* be available on bit 1. If not and it's properly jumpered
;	bit 1 is tied to the drive being selected. The spare bits are tied
;	to 0 and the motor on is 0 when running. In other words we can do
;	the wait purely on flags
;
;	The GM849 moves up to a WD2797 and by this time "normal" drives
;	are 40/80 track. 3.5" and 3" are also supported.
;
;	E4 changes entirely
;	E4 0-2 select drive by value (0-7)
;	E4 3 is now side only
;	E4 4 is SD/DD as before
;	E4 5 is still 5/8" [1 = 8]
;	E4 7 is 300rpm(DD) or 360rpm(HD)
;
;	Basically bit 4/5 are speed selectors
;	5 4
;	0 0	125Kbit FM
;	0 1	250Kbit MFM
;	1 0	250Kbit FM
;	1 1	500Kbit MFM
;
;	And bit 7 controls the rotation speed so you can use 500Kbit MFM
;	with 5.25" to read 1.2MB disks (or 1.44MB) if you have a 4MHz CPU
;
;	E5 bit 7 reads 0 for an 849 and 1 for an 829
;
;
;	The read and write data loops here are taken from
;	"GM809 and Eight Inch Drives" 80BUS News July-Oct 1982, by D Parkinson
;	who deserves a medal for figuring this one out.
;
;	The Nascom FDC is very similar except that E4 is again different
;	0-3: select drive 0-3
;	4: side select (if LK.2 is A-C)
;	5: low if LK3 set (normally high)
;	6: density
;	7: low if LK4 set (normally high)
;
;	Status is on E5 although arranged the same way as the GM8x9
;
	.area _COMMONMEM

FDREAD  .equ 0x88
FDWRITE	.equ 0xA8
FDSEEK	.equ 0x14	; + step rate 0-3  1C to keep head loaded
FDREST	.equ 0x00	;     ""    ""
FDRESCHK .equ 0x04	;     ""    ""	(should this be 0C Check)
;
;	Read the sector data into (HL). Caller has seeked appropriately
;	and loaded sector register
;
_gm8x9_ioread:
	call motorbusy_check
	ret nz
	ld a,(_io_page)
	or a
	call nz, map_process_a
	; FIXME: need to sort out head loading and also side here
	ld a,#FDREAD
	out (0xE0),a			; issue command
	ex (sp),hl			; check length versus 12h djnz loop FIXME
	ex (sp),hl
	ld c,#0xE4			; magic status		
	jr read_sync
read_loop:
	; We can't ldi this because we need C to be E4
	; LDI would be 16 versus 24 but it costs us 8 clocks to move
	; C back and forth so it's not a win
	in a,(0xE3)			; 11
	ld (hl),a			; 7
	inc hl				; 6
read_sync:
	; From the moment DRQ goes true we have 54 T states to read it
	in a,(c)			; 11
	jr z, read_sync			; 12 / 7
	jp m, read_loop			; 10
	; IRQ or timeout (eg motor off) - ie we finished
read_status:
	call map_kernel
	call motor_check
	ret nz
	;
	; Read status
	;
	in a,(0xE0)
	ld l,a
	ret
timed_out:
	ld l,#0xFF
	ret
;
;	Write the sector data to (HL)
;
_gm8x9_iowrite:
	call motorbusy_check
	ret nz
	ld a,(_io_page)
	or a
	call nz, map_process_a
	; FIXME: need to sort out head loading and also side here
	ld a,#FDWRITE
	out (0xE0),a			; issue command
	ex (sp),hl
	ex (sp),hl
	ld c,#0xE4			; 7
write_loop:
	ld a,(hl)			; 7
	inc hl				; 6
write_wait:
	; For write DRQ requires data within 46 T states - hence the
	; load of A must occur first
	in b,(c)			; 12
	jr z, write_wait		; 12 / 7
	; If we load data with an extra byte (as we do on the end), it
	; will be ignored.
	out (0xE3),a			; 11
	jp m, write_loop		; 10
	jr read_status

;
;	On error sets NZ and l to the error code
;
motorbusy_check:
	in a,(0xE0)
	bit 0,a
	jr z,motor_check
	ld l,#253
	ret	
motor_check:
	in a,(0xE4)
	bit 1,a
	ret z
	ld l,#254
	ret

	.area _CODE
;
;	Seek to track L. Assumes motor is running
;
_gm8x9_seek:
	call motorbusy_check
	ret nz
	ld a,l
	out (0xE3),a			; target track
	ld a,#0x01			; always  valid sector
	out (0xE2),a
	ld b,#FDSEEK
issue_seek:
	ld a,(_gm8x9_steprate)
	or b
issue_cmd:
	out (0xE0),a
	ex (sp),hl
	ex (sp),hl
	; FIXME: timeout on the wait loop
seek_wait_ready:
	in a,(0xE4)
	bit 0,a		; check
	jr nz, seek_wait_ready
	in a,(0xE0)
	ld a,l
	ret

;
;	Restore
;
_gm8x9_restore:
	call motorbusy_check
	ret nz
	ld b,#FDREST
	jr issue_seek
;
;	Restore and check we can ready the track
;
_gm8x9_restore_test:
	call motorbusy_check
	ret nz
	ld b,#FDRESCHK		; FIXME - what headload should we have ?
	jr issue_seek

;
;	Reset things
;
;	Needs a timeout check FIXME
;
_gm8x9_reset:
	ld a,#0xD0
reset_wait:
	in a,(0xE4)
	bit 1,a
	jr nz, reset_wait
	ld l,#0
	ret
