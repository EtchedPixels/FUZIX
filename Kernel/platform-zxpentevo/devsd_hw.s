            .module devsd_hw

P_DATA	.equ 0X57
P_CONF	.equ 0X77

SDCTL	.equ 0xFF57
SDDATA	.equ 0x7F57

CMD_09		.equ 0X49	;SEND_CSD
CMD_12		.equ 0X4C	;STOP_TRANSMISSION
CMD_17		.equ 0X51	;READ_SINGLE_BLOCK
CMD_18		.equ 0X52	;READ_MULTIPLE_BLOCK
CMD_24		.equ 0X58	;WRITE_BLOCK
CMD_25		.equ 0X59	;WRITE_MULTIPLE_BLOCK
CMD_55		.equ 0X77	;APP_CMD
CMD_58		.equ 0X7A	;READ_OCR
CMD_59		.equ 0X7B	;CRC_ON_OFF
ACMD_41		.equ 0X69	;SD_SEND_OP_COND


            .area _CODE

;LAST UPDATE: 30.03.2014 savelij

;χθοδξωε παςανετςω οβύιε:
;HL-αδςεσ ϊαηςυϊλι χ πανρτψ
;BCDE-32-θ βιτξωκ ξονες σελτοςα
;A-λομιώεστχο βμολοχ (βμολ=512 βακτ)
;τομψλο δμρ νξοηοβμοώξοκ ϊαπισι/ώτεξιι

;οϋιβλι χωδαχαενωε ξα χωθοδε:
;A=0-ιξιγιαμιϊαγιρ πςοϋμα υσπεϋξο
;A=1-λαςτα ξε ξακδεξα ιμι ξε οτχετιμα

;οβύαρ τοώλα χθοδα δμρ ςαβοτω σ SD

TABLSDZ:	.dw SD_INIT			;0 παςανετςοχ ξε τςεβυετ, ξα χωθοδε A
						;σνοτςι χωϋε πεςχωε 2 ϊξαώεξιρ
		.dw SD_CMP			;1 πςοχεςλα ξαμιώιρ SD λαςτω
		.dw RDMULTI			;2 ώτεξιε σελτοςοχ, λομ-χο χ "A"
		.dw WRMULTI			;3 ϊαπισψ σελτοςοχ

SD_INIT:	CALL CS_HIGH
		LD BC,#P_DATA
		LD DE,#0x10FF
		OUT (C),E
		DEC D
		JR NZ,.-3
		XOR A
		EX AF,AF'	;'
ZAW001:		LD HL,#CMD00
		CALL OUTCOM
		CALL IN_OOUT
		EX AF,AF'	;'
		DEC A
		JR Z,ZAW003
		EX AF,AF'	;'
		DEC A
		JR NZ,ZAW001
		LD HL,#CMD08
		CALL OUTCOM
		CALL IN_OOUT
		IN H,(C)
		NOP
		IN H,(C)
		NOP
		IN H,(C)
		NOP
		IN H,(C)
		LD HL,#0
		BIT 2,A
		JR NZ,ZAW006
		LD H,#0x40
ZAW006:		LD A,#CMD_55
		CALL OUT_COM
		CALL IN_OOUT
		LD A,#ACMD_41
		OUT (C),A
		NOP
		OUT (C),H
		NOP
		OUT (C),L
		NOP
		OUT (C),L
		NOP
		OUT (C),L
		LD A,#0xFF
		OUT (C),A
		CALL IN_OOUT
		AND A
		JR NZ,ZAW006
ZAW004:		LD A,#CMD_59
		CALL OUT_COM
		CALL IN_OOUT
		AND A
		JR NZ,ZAW004
ZAW005:		LD HL,#CMD16
		CALL OUTCOM
		CALL IN_OOUT
		AND A
		JR NZ,ZAW005
CS_HIGH:	PUSH AF
		LD A,#3
		OUT (P_CONF),A
		XOR A
		OUT (P_DATA),A
		POP AF
		RET

ZAW003:		CALL SD__OFF
		INC A
		RET

SD__OFF:	XOR A
		OUT (P_CONF),A
		OUT (P_DATA),A
		RET

CS__LOW:	PUSH AF
		LD A,#1
		OUT (P_CONF),A
		POP AF
		RET

OUTCOM:		CALL CS__LOW
		PUSH BC
		LD BC,#0X0600+P_DATA
		OTIR
		POP BC
		RET

OUT_COM:	PUSH BC
		CALL CS__LOW
		LD BC,#P_DATA
		OUT (C),A
		XOR A
		OUT (C),A
		NOP
		OUT (C),A
		NOP
		OUT (C),A
		NOP
		OUT (C),A
		DEC A
		OUT (C),A
		POP BC
		RET

SD_CMP:		LD A,#CMD_58
		LD BC,#P_DATA
		CALL OUT_COM
		CALL IN_OOUT
		IN H,(C)
		NOP
		IN H,(C)
		NOP
		IN H,(C)
		NOP
		IN H,(C)
		RET

SECM200:	PUSH HL
		PUSH DE
		PUSH BC
		PUSH AF
		PUSH BC
		LD A,#CMD_58
		LD BC,#P_DATA
		CALL OUT_COM
		CALL IN_OOUT
		IN A,(C)
		NOP
		IN H,(C)
		NOP
		IN H,(C)
		NOP
		IN H,(C)
		INC A
;--sfs		JP Z,SD_CARD_LOST
		DEC A
		BIT 6,A
		POP HL
		JR NZ,SECN200
		EX DE,HL
		ADD HL,HL
		EX DE,HL
		ADC HL,HL
		LD H,L
		LD L,D
		LD D,E
		LD E,#0
SECN200:	POP AF
		LD BC,#P_DATA
		OUT (C),A
		NOP
		OUT (C),H
		NOP
		OUT (C),L
		NOP
		OUT (C),D
		NOP
		OUT (C),E
		LD A,#0xFF
		OUT (C),A
		POP BC
		POP DE
		POP HL
		RET

IN_OOUT:	PUSH DE
		LD DE,#0x30FF
IN_WAIT:	IN A,(P_DATA)
		CP E
		JR NZ,IN_EXIT
IN_NEXT:	DEC D
		JR NZ,IN_WAIT
IN_EXIT:	POP DE
		RET

CMD00:		.db 0X40,0X00,0X00,0X00,0X00,0X95		;GO_IDLE_STATE
CMD08:		.db 0X48,0X00,0X00,0X01,0XAA,0X87		;SEND_IF_COND
CMD16:		.db 0X50,0X00,0X00,0X02,0X00,0XFF		;SET_BLOCKEN

RD_SECT:		PUSH BC

;		LD A,IYL
;		.db 0xFD
;		ld a,l
		;
		BIT 1,A
		JR NZ,RD_SECT3			;εσμι δμρ χξεϋξεκ 
		AND A
		JR NZ,RD_SECT2
RD_SECT3:
;		LD A,(R_7FFD)

;		LD BC,(B0_CPU2)
		JR Z,RD_SECT1
;		LD BC,(B1_CPU2)
RD_SECT1:	
;		PEC_ON SHADOW_BF
;		LD A,B
;		OR 0X37
;		LD B,A
;		LD A,C
;		LD C,LOW (WIN_A0)
;		OUT (C),A
;		PEC_OFF SHADOW_BF
RD_SECT2:	LD BC,#P_DATA
		INIR
		NOP
		INIR
		NOP
		IN A,(C)
		NOP
		IN A,(C)
;		PEC_ON SHADOW_BF
;		LD BC,WIN_P6
;		XOR A
;		OUT (C),A
;		PEC_OFF SHADOW_BF
		POP BC
		RET

WR_SECT:	PUSH BC
		LD BC,#P_DATA
		OUT (C),A
		OTIR
		NOP
		OTIR
		LD A,#0xFF
		OUT (C),A
		NOP
		OUT (C),A
		POP BC
		RET

RDMULTI:	EX AF,AF'	;'
		LD A,#CMD_18
		CALL SECM200
		EX AF,AF'	;'
RDMULT1:	EX AF,AF'	;'
		CALL IN_OOUT
		CP #0xFE
		JR NZ,.-5
		CALL RD_SECT
		EX AF,AF'	;'
		DEC A
		JR NZ,RDMULT1
		LD A,#CMD_12
		CALL OUT_COM
		CALL IN_OOUT
		INC A
		JR NZ,.-4
		JP CS_HIGH

WRMULTI:	EX AF,AF'	;'
		LD A,#CMD_25
		CALL SECM200
		CALL IN_OOUT
		INC A
		JR NZ,.-4
		EX AF,AF'	;'
WRMULT1:	EX AF,AF'	;'
		LD A,#0xFC
		CALL WR_SECT
		CALL IN_OOUT
		INC A
		JR NZ,.-4
		EX AF,AF'	;'
		DEC A
		JR NZ,WRMULT1
		LD C,#P_DATA
		LD A,#0xFD
		OUT (C),A
		CALL IN_OOUT
		INC A
		JR NZ,.-4
		JP CS_HIGH
