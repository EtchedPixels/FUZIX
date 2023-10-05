;
;	We are run from the ROM start of the cartridge. Because this is a
;	TS2068/TC2068 the DivIDE will have the ROM paging disabled
;
	.area _CODE


	.globl _start

	.globl boot

;
;	From 0x8000
;
;	AROS header

	.byte 0x02		; Machine code
	.byte 0x02		; AROS
	.word boot		; address to run
	.byte 0x0f		; page us in the top 32K please (our ROM)
	.byte 0x01		; autostart
	.word 0x0021		; reserve us no memory (yet another crappy
				; 2068 bug)
boot:
	; We are entered with cartridge ROM in the top 32K, system RAM in
	; the low 32K

	di			; we are going to blow stuff away
	; Hide the screen contents
	ld hl,#0x5800
	ld de,#0x5801
	ld bc,#0x02FF
	ld (hl),#0
	ldir

	ld sp,#0x8000		; We don't load 7E00-7FFF right now

	; Black border
	xor a
	out (254),a

	;
	;	The kernel needs cartridge RAM mapped in 0x0000-7FFF
	;	and cartridge ROM to 8000-FFFF
	;	
	ld a,#0xFF
	out (0xF4),a	

	;
	;	Ensure the master drive is selected
	;
wait1:
	in a,(191)
	rla
	jr c, wait1
	ld a,#0xE0
	out (187),a
	nop
wait2:
	in a,(191)
	and #0xC0
	cp #0x40		; want busy off, drdy
	jr nz, wait2

	;
	; Load sectors. We shortcut stuff here because we never
	; load over 256 sectors
	;
	; Most stuff is in ROM already. We just need to load the
	; remaining chunk from 0000-3FFF and 5B00-7FFF (and it's easier
	; just to load through the screen)
	;
	xor a			; LBA28 high bits
	out (179),a
	out (183),a

	ld de,#0x3F01		; Load 63 sectors
				; from sector 1
	ld hl,#0x0000		; Starting address to load

load_loop:
	ld a,e
	inc e
	out (175),a		; sector number to load
	ld a,#1			; load one sector
	out (171),a
	ld a,#0x20		; READ
	out (191),a
	nop
wait3:
	in a, (191)
	rlca
	jr c, wait3		; Busy
	bit 4,a			; DRQ ?
	jr z, failed		; Nope - bad
	ld bc,#163		; Data port
	inir
	inir
	dec d
	jr nz, load_loop

	; And we are done

	jp _start		; Enter entirely cartridge mapped
failed:
	ld a,#0x05
	out (0xFF),a
	di
	halt

;
;	Zero page
;
	.area _PAGE0	(ABS)

	.globl null_handler
	.globl unix_syscall_entry
	.globl interrupt_handler

	.org 0
loader:
	jp boot
	.ds 5
rst_8:
	.ds 8
rst_10:
	.ds 8
rst_18:
	.ds 8
rst_20:
	.ds 8
rst_28:
	.ds 8
rst_30:
	jp unix_syscall_entry
	.ds 5
rst_38:
	jp interrupt_handler
	.ds 0x66-0x3B
nmi:	ret		; magic...
	retn

