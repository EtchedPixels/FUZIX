;
;	ROMWBW test
;
		.module boot
; MMU Ports
MPGSEL_0	.equ	0x78	; Bank_0 page select register (W/O)
MPGSEL_1	.equ	0x79	; Bank_1 page select register (W/O)
MPGSEL_2	.equ	0x7A	; Bank_2 page select register (W/O)
MPGSEL_3	.equ	0x7B	; Bank_3 page select register (W/O)
MPGENA		.equ	0x7C	; memory paging enable register, bit 0 (W/O)


.area	    BOOT	(ABS)
	.org	    0xF000

	.ds   384
	.byte 0x5A
	.byte 0xA5
	.byte 0x00	; formatting platform
	.byte 0x00	; device
	.byte 0x00	; formatter
	.byte 0x00	; physical drive
	.byte 0x00	; logical unit
	.byte 0x00	; unused
	.ds 88		; move to byte 96
	.byte 0x00	; write protect
	.word 0x00	; update count
	.byte 0		; major
	.byte 0		; minor
	.byte 0		; update
	.byte 0		; patch
	.ascii 'Fuzix WBW Loader'
	.byte '$'	; indicate valid label
	.word 0		; no patch
	.word 0xF200	; load target
	.word 0xF400	; end - way more than we need but that's fine
	.word 0xF200	; run from here

;
;	This should be 0xF200. We are entered with the stack somewhere
;	in the middle of RAM, HBIOS stubs in the top 2 pages and RAM
;	low including our expected RST hooks for HBIOS
;
;	We need to go get our device/subunit back from the BIOS because
;	it's has RST 8 interfaces for this rather than just passing
;	them in a register.
;
bootit:
	ld sp, #0xFE00		; SP as high as we can
;
;	We make a few assumptions in order to try and be strong against
;	problems
;
	di
	ld hl,#serial
	call strout

	ld bc,#(32*256+MPGSEL_1)	; b = bank c = port
testnext:
	ld hl,#dot
	call strout

	out (c),b
	ld a,#0xAA
	ld hl,#0x4000
	;
	;	We touch every byte of RAM so don't use the stack
	;	in this sequence
	;
testram:
	ld a,#0xAA
	ld (hl),a
	cp (hl)
	jr nz, badram
	cpl
	ld (hl),a
	cp (hl)
	jr nz, badram
	inc hl
	bit 7,h			; H > 0x7F
	jr z,testram
	inc b
	ld a,#63		; We are in bank 63 so we can't test it
	cp b
	jr nz,testnext
	ld sp,#0xC000

;
;	Make sure everything from here is position independent
;
testalias:
	ld hl,#mpgchk
	call strout
	ld bc,#MPGSEL_0
	ld hl,#0x0000
testnextmpg:
	ld b,#32
testaliasn:
	; Write the bank number into each bank
	out (c),b
	ld (hl),b
	inc b
	bit 6,b
	jr z,testaliasn
	ld b,#32
chkaliasn:
	out (c),b
	ld a,(hl)
	cp b
	jr nz,badmpg
	inc b
	bit 6,b
	jr z,chkaliasn
	inc c
	ld a,h
	add #0x40
	ld h,a
	ld a,c
	cp #MPGSEL_3
	jr c,testnextmpg
;
;	MPGSEL_3 is a bit harder to test as our code is in it
;
;	We've tested RAM and lower MPGSEL so use one of those
;
;	C meant we were below MPGSEL_3, Z means we are on it, NZ
;	means we finished running the copy
;
	ret nz
;
	ld a,#32
	out (MPGSEL_0),a
	ld hl,#testnextmpg
	ld de,#0x0100
	ld bc,#0x0200
	ldir
	ld c,#MPGSEL_3
	call 0x0100
	ld hl,#passed
done:
	call strout
	di
	halt
badmpg:
	ld hl,#mpgbad
	jr done
badram:
	ld hl,#rambad
	jr done
strout:
	xor a
	out (0x81),a
	in a,(0x81)
	and #4
	jr z,strout

	ld a,(hl)
	or a
	ret z
	out (0x80),a
	inc hl
	jr strout
	

serial:
	.byte 13,10
	.asciz 'Starting RAM test: '
dot:
	.asciz '.'
mpgchk:
	.byte 13,10
	.asciz 'Testing MPGSEL'
passed:
	.byte 13,10
	.ascii 'Passed'
	.byte 13,10,0
mpgbad:
	.byte 13,10
	.ascii 'Fail'
	.byte 13,10,0
rambad:
	.byte 13,10
	.ascii 'Bad RAM'
	.byte 13,10,0
