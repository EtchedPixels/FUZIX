;
;	DivIDE is a bit of a pain as all the firmware is designed around
;	ROM BASIC and snapshots. We trick it by providing a fake BOOT.BIN.
;	Fatware thinks that this is the Fatware code it wants to put in
;	bank3 and then lock. In fact it's our loader which it will load
;	and lock for us.
;
	.area BOOT	(ABS)

	.globl null_handler
	.globl interrupt_handler
	.globl ___sdcc_enter_ix

	.org 0

;
;	We don't have a JP at 0 as we'd like so our low level code needs
;	to avoid that check
;
loader:
	di
	ld a,#0x80
	out (0xE3),a
	; Page the EEPROM in and control transfers there
	; not to the jp below
loader5:
	; This is where the EEPROM calls us
	jp start
rst_8:
	jp	___sdcc_enter_ix
	.ds	5
rst10:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
rst18:
	pop	af
	pop	ix
	ret
	.ds	4
rst20:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
	.ds 3
rst_28:
	.ds 8
rst_30:
	.ds 8
rst_38:
	jp interrupt_handler
	.ds 0x66-0x3B
nmi:	ret		; magic...
	retn

;
;	Load and go
;
start:
	di
	ld bc,#0x7ffd

	; Map some RAM (must not be RAM2) in 2000-3FFF
	ld a,#0x40
	out (0xE3),a

	; Stack in the first 512 bytes of our data space
	ld sp,#0x2200

	; Select the shadow screen bank
	ld a,#0x07
	out (c), a

	; Clear the shadow screen
	ld hl,#0xc000
	ld de,#0xc001
	ld bc,#6144
	ld (hl),#0
	ldir
	; Attributes to green writing on black
	ld (hl),#4
	ld bc,#767
	ldir

	; Black border
	xor a
	out (254),a

	; Shadow screen on, bank 0 back at C000-FFFF		
	ld bc,#0x7ffd
	ld a,#0x18
	out (c),a

	; Screen is now on bank 7
	; Memory is on bank 0

	; Put some RAM in 0x2000-3FFF and keep us locked in the low 8K
	; Do not use #0x42, this is magic for allram mode on later DivIDE
	ld a,#0x40
	out (0xE3),a
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
	;
	; We load 112 sectors into 2000-FFFF using bank 0 as the top bank
	; We then load 32 sectors into the 16K at C000-FFFF bank 1
	; And finally the same for bank 7
	; 0x2000 should start ZB then the execution address
	;
	; For simplicity we don't bother with a stack, we just use IX
	;
	xor a			; LBA28 high bits
	out (179),a
	out (183),a
	ld de,#0x6F01		; Load 111 sectors (2200-FFFF)
				; from sector 1
	ld hl,#0x2200		; Starting address to load
	call load_loop

	ld a,#0x19		; Select bank 1
	ld bc,#0x7ffd
	out (c),a
	ld d,#0x20		; Load 32 sectors (C000-FFFF)
	ld hl,#0xc000
	call load_loop

	ld a,#0x1F		; Select bank 7
	ld bc,#0x7ffd
	out (c),a
	ld d,#0x20		; Load 32 sectors (C000-FFFF)
	ld hl,#0xC000
	call load_loop

	ld a,#0x18
	ld bc,#0x7ffd
	out (c),a		; Switch back to bank 0
	ld hl,#0x2200
	; 0x2200 should start with a signature of ZB then the execute
	; address
	ld a,(hl)
	cp #'Z'
	jr nz, failed
	inc hl
	ld a,(hl)
	cp #'B'
	jr nz, failed
	inc hl
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a
	jp (hl)

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
	ret

failed:
	ld hl,#1
	jp OutJPHL		; Into Spectrum ROM and reboot

	.area BOOT1FEC

	;
	;	This has to match the ROM expecations
	;
	out (0xe3),a
	pop af
	ret

	.area BOOT1FF7

	;
	; For compatibility with FatWare
	;
OutEI:	ei
OutRet:	ret
OutJPHL:	jp (hl)

	.area BOOT1FFE
RomSign:
	.db 0,14
