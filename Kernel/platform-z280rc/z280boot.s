; 5/19/18 ZZMon is relocated to 0xB400.  CFMon will be loaded to 0xC000.
; This is the cold bootstrap that loads the CF copying program to 0x1000 and then jump to 0x1000. 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CFMonLdr, Copyright (C) 2018 Hui-chien Shen
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	Converted to SDCC asm quirkyness - Alan Cox
;

	ld hl,#0xc000	; create CFMon in 0xC000
        ld (hl),#0x3e; op code for LD A,40h
	inc hl
	ld (hl),#0x40
	inc hl
        ld (hl),#0xd3; op code for OUT (CF2427),A
	inc hl
        ld (hl),#0x0cd
	inc hl
        ld (hl),#0x16; op code for LD D,0F8h
	inc hl
	ld (hl),#0xf8
	inc hl
        ld (hl),#0x21; op code for LD HL,0B400h
	inc hl
	ld (hl),#0x00
	inc hl
	ld (hl),#0xb4
	inc hl
        ld (hl),#0x0e;op code for LD C,CFdata
	inc hl
	ld (hl),#0xc0
	inc hl
        ld (hl),#0x3e;op code for LD A,1
	inc hl
	ld (hl),#0x01
	inc hl	
        ld (hl),#0x0d3;op code for OUT (CFsectcnt),A
	inc hl
	ld (hl),#0xc5
	inc hl	
        ld (hl),#0x7a;op code for LD A,D
	inc hl
	ld (hl),#0xfe;op code for CP 0FEh
	inc hl	
        ld (hl),#0xfe
	inc hl
	ld (hl),#0xca;op code for JP Z,0B400h
	inc hl
        ld (hl),#0x00
	inc hl
	ld (hl),#0xb4
	inc hl
        ld (hl),#0xd3;op code for OUT (CF07),A
	inc hl
	ld (hl),#0xc7
	inc hl
        ld (hl),#0x3e;op code for LD A,20h
	inc hl
	ld (hl),#0x20
	inc hl	
        ld (hl),#0xd3;op code for OUT (CFstat),A
	inc hl
	ld (hl),#0xcf
	inc hl
	ld (hl),#0xdb;op code for IN A,(CFstat)
	inc hl
	ld (hl),#0xcf
	inc hl
	ld (hl),#0xe6;op code for AND 80h
	inc hl
	ld (hl),#0x80
	inc hl
	ld (hl),#0xc2;op code for JP NZ,0C01bh
	inc hl
	ld (hl),#0x1b
	inc hl
	ld (hl),#0xc0
	inc hl
	ld (hl),#0xdb;op code for IN A,(CFstat)
	inc hl
	ld (hl),#0xcf
	inc hl
        ld (hl),#0xe6;op code for AND 8
	inc hl
	ld (hl),#0x08
	inc hl		
        ld (hl),#0xca;op code for JP Z,0C022h
	inc hl
	ld (hl),#0x22
	inc hl
        ld (hl),#0xc0
	inc hl
	ld (hl),#0x06;op code for LD B,0h
	inc hl
        ld (hl),#0x00
	inc hl
	ld (hl),#0xed;op code for INIRW
	inc hl
        ld (hl),#0x92
	inc hl
	ld (hl),#0xdb;op code for IN A,(CFstat)
	inc hl	
        ld (hl),#0xcf
	inc hl
	ld (hl),#0x14;op code for INC D
	inc hl	
        ld (hl),#0xc3;op code for JP 0C00bh
	inc hl
	ld (hl),#0x0b
	inc hl		
        ld (hl),#0xc0

	jp 0xc000	; start CFMon execution at 0xC000

; This is the CFMon program that'll be created in 0xC000 by the CFMonLdr
;
;	                             A       23 	org 0C000h
;	0000C000 3E 40               A       25 	ld a,0x40	; LA addressing mode
;	0000C002 D3 CD               A       26 	out (CF2427),a
;	0000C004 16 F8               A       27 	ld d,0xf8	; points to sector to read, last 8 sectors in track 0
;	0000C006 21 00 B4            A       28 	ld hl,0xb400; store CF data starting from 0b400h
;	0000C009 0E C0               A       29 	ld c,CFdata	; reg C points to CF data reg
;	0000C00B                     A       30 moresect:
;	0000C00B 3E 01               A       31 	ld a,1		; read 1 sector
;	0000C00D D3 C5               A       32 	out (CFsectcnt),a	; write to sector count with 1
;	0000C00F 7A                  A       33 	ld a,d		; read sector pointed by reg D
;	0000C010 FE FE               A       34 	cp 0xfe	; read sectors 0xF8-0xFD
;	0000C012 CA 00 B4            A       35 	jp z,0xb400	; load completed, run program loaded at 0xB400
;	0000C015 D3 C7               A       36 	out (CF07),a	; read the sector pointed by reg D		
;	                             A       37 ; no need to setup track, it is setup by CFinit state machine
;	                             A       38 ;	xor a		; clear reg A
;	                             A       39 ;	out (CF1623),a	; track 0
;	                             A       40 ;	out (CF815),a
;	0000C017 3E 20               A       41 	ld a,0x20	; read sector command
;	0000C019 D3 CF               A       42 	out (CFstat),a	; issue the read sector command
;	0000C01B                     A       43 readbsy:
;	0000C01B DB CF               A       44 	in a,(CFstat)	; check bsy flag first
;	0000C01D E6 80               A       45 	and 0x80	; bsy flag is at bit 7
;	0000C01F C2 1B C0            A       46 	jp nz,readbsy
;	0000C022                     A       47 readdrq:
;	0000C022 DB CF               A       48 	in a,(CFstat)	; check data request bit set before read CF data
;	0000C024 E6 08               A       49 	and 0x08	; bit 3 is DRQ, wait for it to set
;	0000C026 CA 22 C0            A       50 	jp z,readdrq
;	                             A       51 
;	0000C029 06 00               A       52 	ld b,0	; sector has 256 16-bit data
;	0000C02B ED 92               A       53 	db 0xed,0x92; op code for inirw input word and increment
;	                             A       54 ;	inirw		; reg HL and c are already setup at the top
;	0000C02D DB CF               A       55 	in a,(CFstat)	; OUTJMP bug fix
;	0000C02F 14                  A       56 	inc d
;	0000C030 C3 0B C0            A       57 	jp moresect
;