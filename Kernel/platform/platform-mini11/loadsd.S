;
;	The SD boot code hands us
;	A = card type
;	X = I/O base ($F000)
;	Y = our base
;	S = internal RAM somewere at F0FF or so
;	P = interrupts off
;
;	IO at F000, IRAM at F000 (with some overlap)
;

#define	CTMMC	1
#define CTSD2	2
#define CTSDBLK 3
#define CTSD1	4

#define SPCR	0x28
#define SPSR	0x29
#define SPDR	0x2A
#define PDDR	0x08
#define DDRD	0x09

	FDB	0x6811
START:
	BRA GO

LBAINC:	FDB	0x200
CARDTYPE:
	FCB	0x00
CMD17:
	FCB 0x51,0,0,0,0,0x01

GO:
	; Block or byte LBA - set LBAINC accordingly
	STAA CARDTYPE
	CMPA #CTSDBLK
	BNE BYTEMODE
	LDD #1
	STD LBAINC
BYTEMODE:
	LDAA #0x50		; SPI on master, faster
	STAA SPCR,X

	LDAA #0x88
	STAA ,X			; ROM off bank 0, CS1 high

	; Load from 0x0200
	LDY #0x0200

LOADLOOP:
	PSHY			; Save pointer whist we do the command
	LDY #CMD17
	; Move on an LBA block
	LDD 3,Y			; Update the offset or LBA number
	ADDD LBAINC
	STD 3,Y
	JSR SENDCMD		; Send a read command
	BNE SDFAIL
WAITDATA:
	BSR SENDFF		; Wait for the FE marker
	CMPB #0xFE
	BNE WAITDATA
	PULY			; Recover data pointer
	CLRA			; Copy count (512 bytes)
DATALOOP:
	BSR SENDFF
	STAB ,Y
	BSR SENDFF
	STAB 1,Y
	INY
	INY
	DECA
	BNE DATALOOP
	BSR CSRAISE		; End command
	LDAA #'.'
	BSR OUTCH
	CPY #0x0000
	BEQ LOADDONE
	CPY #0xF000
	BNE LOADLOOP		; Done ?
	PSHY			; Save address
	;	Skip the block F000-F1FF (I/O space and IRAM)
	LDY #CMD17
	; Move on an LBA block
	LDD 3,Y			; Update the offset or LBA number
	ADDD LBAINC
	STD 3,Y
	PULY
	XGDY
	ADDD #0x0200
	XGDY
	BRA LOADLOOP

LOADDONE:
	LDAA #0x0D
	BSR OUTCH
	LDAA #0x0A
	BSR OUTCH
	JMP 0x0200		; And run

SDFAIL: LDAA #'E'
FAULT:	BSR OUTCH
STOPB:	BRA STOPB

OUTCH:
	BRCLR 0x2E,X 0x80 OUTCH
	STAA 0x2F,X
	RTS

CSLOWER:
	BCLR PDDR,X 0x20
	RTS
;
;	This lot must preserve A
;
CSRAISE:
	BSET PDDR,X 0x20
SENDFF:
	LDAB #0xFF
SEND:
	STAB SPDR,X
SENDW:
	BRCLR SPSR,X 0x80 SENDW
	LDAB SPDR,X
	RTS

SENDCMD:
	BSR CSRAISE
	BSR CSLOWER
WAITFF:
	BSR SENDFF
	INCB
	BNE WAITFF
NOWAITFF:
	; Command, 4 bytes data, CRC all preformatted
	LDAA #6
SENDLP:
	LDAB ,Y
	BSR SEND
	INY
	DECA
	BNE SENDLP
	BSR SENDFF
WAITRET:
	BSR SENDFF
	BITB #0x80
	BNE WAITRET
	CMPB #0x00
	RTS
