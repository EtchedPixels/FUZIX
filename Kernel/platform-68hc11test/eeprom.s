;
;	EEPROM firmware for the 68HC811E2
;
;	TODO
;	- any debug/testing 8)
;
;	Add functions for spi block copy to/from banks
;	Add description of SPI attached devices for OS
;	Make serial mode ask if you want serial or retry SD
;	Set up RTI timer and use interrupt to trap out of SPI layer
;	on a timeout
;
;	Export outchar/outstr and friends so OS can use them (duplicated
;	right now). Ditto for inchar, inchar/wait type stuff
;
;	Provide 'read block' 'write block' block device helpers so that
;	the boot block can be tighter ?
;
;
	.mode mshort

	.globl _start

	include "eeprom.def"

	.sect .text
;
_start:
	lds #0xF100	;	Stack in unbanked internal ram
	ldaa #0xFF
	staa init	;	Move the internal RAM and I/O to Fxxx
	stab porta	;	clear output controls
	ldaa #0x88
	staa pactl	;	Port A bit 7 and bit 3 to output
;
;	Real RAM is now definitely in bank 0
;
	ldaa #0x30	;	9600 baud
	staa baud
	ldy #boot1	;	say hello
	jsr outstr
	ldd #0xff55
	ldy #0x1000	
	staa ,y		;	arbitrary real RAM if we set up right
	cmpa ,y		;	should match
	bne ramfucked
	dec ,y		;	zero
	tst ,y		;	check
	bne ramfucked
;
;	OK we seem to have RAM wired to do 00 and FF
;
	stab 1,y	;	0x55
	ldaa ,y		;	should be unchanged
	cmpa #0xff	;	address bus not working ?
	bne ramfucked2
;
;	Promising !
;
	clrb
writepgs:		;	write the page ident into each bank
	stab ,x		;	select this RAM bank
	stab ,y
	addb #0x08
	cmpb #0xF8
	bne writepgs
	clrb
readpgs:		;	read the page ident from each bank
	stab ,x		;	select this RAM bank
	cmpb ,y		;	should match
	bne ramfucked3	;	bank bits busted
	addb #0x08
	cmpb #0xF8
	bne readpgs
;
;	We could size the RAM if we needed to be flexible in future but
;	for now we don't so cheat
;
	ldy #boot2
	jsr outstr

	ldx #rambase
	ldd #0xC0
wipelow:
	staa ,x
	inx
	deca
	bne wipelow

	; Save RAM size (fixed for now)
	ldd #512
	std ram
	; Set for board (FIXME - .equ for this)
	ldd #2048
	std eclock
;
;	Serial works, RAM seems to work, it's time to whack the SD card
;
	jsr sdboot
;
;	If not then we try a serial boot
;
	jsr serboot
	ldx #bootfail
	jsr outstr
	ldaa #0xA0
	bra fail

ramfucked:
	ldaa #0x88
	bra fail
ramfucked2:
	ldaa #0x90
	bra fail
ramfucked3:
	ldaa #0x98
fail:
	jsr outcharhex		; print the hex code
	staa ,x			; assert it on the bank control pins
failloop:
	staa 0xDEAD		; keep asserting DEAD on the address bus
	bra failloop		; and the error on data

outcharhex:
	pshb
	lsrb
	lsrb
	lsrb
	lsrb
	bsr outnibble
	pulb
	pshb
	bsr outnibble
	pulb
	rts

outnibble:
	andb #0x0F
	cmpb #0x0A
	ble outh2
	addb #0x07
outh2:	addb #0x30
outchar:
	psha
outchar1:
	ldaa scsr
	anda #0x80
	beq outchar1
	stab scdr
	pula
outsdone:
	rts

outstr:
	ldab ,x
	beq outsdone
	bsr outchar
	inx
	bra outstr

;
;	Send an extended command - CMD 55 then the actual cmd. If the CMD55
;	is rejected the then ACMD is rejected
;
spi_send_acmdxy:
	pshb
	pshx
	pshy
	ldab #55
	bsr spi_send_cmd
	cmpb #1
	bgt acmd_fail
	puly
	pulx
	pulb
	bra spi_send_cmd
acmd_fail:
	rts
;
;	Read a byte, only uses B
;
spi_rx:
	ldab #0xFF
spi_tx:
	stab spdr
spi_rx_w:
	ldab spsr
	andb #0x80
	beq spi_rx_w		; Fixme - timeout handling
	ldab spdr
	rts

sd_spi_wait_ff:			; Timeouts!
	bsr spi_rx
	cmpb #0xff
	beq sd_spi_wait_ff
	rts

sd_spi_wait:			; Ditto
	bsr spi_rx
	cmpb #0xff
	bne sd_spi_wait
	rts

;
;	Chip selects for SD card
;
spi_cs_hi:
	psha			; other bits are not write bits so be slack
	ldaa #0x40
	staa portd
	pula
	rts

spi_cs_lo:
	clr portd
	rts

spi_send_cmd:
	ldx #0
spi_send_cmdx:
	ldy #0
spi_send_cmdxy:
	addb #0x40		; command number to code
	bsr spi_cs_hi
	bsr spi_rx		; clocks
	bsr spi_cs_lo
	bsr sd_spi_wait_ff
	tsta
	bne sdfail
	pshb
	bsr spi_tx		; send acc B
	xgdy
	pshb
	tab
	bsr spi_tx
	pulb
	bsr spi_tx
	xgdx
	pshb
	tab
	bsr spi_tx
	pulb
	bsr spi_tx
	pulb			; get the cmd back
	pshb
	ldaa #0x01		; dummy + stop
	cmpb #0x40		; special cases
	bne ncrc_cmd0
	ldaa #0x95
	bra tx_crc
ncrc_cmd0:
	cmpb #0x48
	bne tx_crc
	ldaa #0x87
tx_crc:	tab
	bsr spi_tx
	pulb
	cmpb #0x4C
	bne no_rx_dummy
	bsr spi_rx
no_rx_dummy:
	ldaa #0x14
rx_waitr:
	bsr spi_rx
	bitb #0x80
	beq cmd_rx_done
	deca
	bne rx_waitr
cmd_rx_done:
	rts


sdfail:
	ldab #0xFF
	rts
sdboot:
	ldd #0x523F
	staa spcr		; SPI on, master, SPI(0,0), 125KHz
	stab ddrd		; Port 5 is output (not !SS)
	bsr spi_cs_hi
	ldaa #0x20
sdboot1:
	bsr spi_rx		; send clocks
	deca
	bne sdboot1
	clrb			; CMD 0
	bsr spi_send_cmd
	cmpa #1			; 1 indicates OK
	bne sdfail
	ldx #spi_probe1
	jsr outstr		; display indication of probing
				; as we have a card of some form
	ldab #8
	ldx #0x01AA
	bsr spi_send_cmdx
	cmpb #1			; if CMD 8 fails it's not SDHC
	bne try_mmc
	;
	;	Looks like we are an SDHC card
	;
	ldx #spibuf
	ldab #4
	jsr spi_rx		; should be four data bytes
	ldd #0x01AA
	cmpa spibuf+2		; and should end with 0x01AA
	bne bad_card_v		; bad voltage ?
	cmpb spibuf+3
	bne bad_card_v
	;
	;	Ok looks good now see if it will wake up
	;	FIXME: timeout needed
	;
spinidle:
	ldy #0x4000
	ldx #0
	ldab #41
	jsr spi_send_acmdxy	; wait for the card to exit idle
	tsta
	bne spinidle
	ldab #58
	jsr spi_send_cmd
	ldab #4
	ldx #spibuf
	jsr spi_rx
	ldab spibuf
	ldaa #BDEV_SD2
	andb #0x40
	beq notblock
	oraa #BDEVF_LBA
notblock:
	staa blocktype
	ldx #boot_sdhc
	jsr outstr
	bra sd_valid

bad_card_v:
	ldx #spi_badcardv
	jmp outstr

try_mmc:
	ldab #41
	ldx #0
	ldy #0
	jsr spi_send_acmdxy
	cmpa #1
	ble is_sdv1
	ldx #boot_mmc
	jsr outstr
	ldaa #BDEV_MMC
	bra det_ok
is_sdv1:
	ldx #boot_sdv1
	jsr outstr
	ldaa #BDEV_SD1
det_ok:
	staa blocktype
sd_valid:
	ldaa #0x50
	staa spcr		; SPI to full speed

	ldx #0			; Block or byte 0 - we don't care about LBA!
	ldy #0
	ldab #17		; Read
	jsr spi_send_cmdxy
	tsta			; Read command worked
	bne spi_rx_fail
	ldd #0xFE		; FIXME: check
	jsr sd_spi_wait
	tsta
	bne spi_rx_fail
	ldx #0xE000
	ldab #0x00
sd_rx_l:
	staa spdr		; any value will do
sd_rx_w:
	ldaa spcr		; are we done yet
	anda #0x80
	beq sd_rx_w
	ldaa spdr		; SPI data byte
	staa ,x
	inx
	decb
	bne sd_rx_l
;
;	Success
;
	ldx 0xE000
	cpx #0xC0DE
	bne notbootable
	ldx #sdbooting
	jsr outstr
	jmp 0xE002		; jmp to boot block

spi_rx_fail:
	ldx #ioerror
	bra sdretry

notbootable:
	ldx #notbootmedia
sdretry:
	jsr outstr
	jsr hitnl
	jmp sdboot		; try again
	

;
;	Serial input
;

hitnl:	bsr waitkey
	cmpa #13
	beq nlok
	cmpa #10
	bne hitnl
nlok:	rts

waitkey:
	ldab scsr
	andb #0x20
	beq waitkey
	ldab scdr
	rts

serboot:
	ldx #sergo
	jsr outstr
	ldx #0xE000
serbootl:
	bsr waitkey
	stab ,x
	inx
	cmpx #0xE200
	bne serbootl
	ldd #0xE000
	cmpa #0xC0
	bne serbad
	cmpb #0xDE
	bne serbad
	jmp 0xE002
serbad:
	rts

;
;	Service Routines.
;
bankcomp:
	ldab porta
	pshb
	andb #0xC0		; preserve other controls
	aba			; we know the bank in A will have the other
				; bits clear
	pulb			; B gets us back
	rts
	
fargetw:
	bsr bankcomp
	staa porta		; switch bank
	ldx  ,x			; get the word
	stab porta 		; and back
	xgdx			; result in D
	rts

fargetb:
	bsr fargetw
	tab			; big endian so we want high half in low
	clra
	rts

farputw:
	bsr bankcomp
	staa porta
	sty ,x
	stab porta
	rts

farputb:
	xgdy
	bsr bankcomp
	staa porta
	xgdy			; b is now the value
	stab ,x
	xgdy			; b is now the bank restore
	stab porta
	rts

; FIXME - optimise for 2 bytes at a time ?

farzero:
	bsr bankcomp
	staa porta
farzl:
	clr ,x
	inx
	dey
	cmpy #0
	bne farzl
	stab porta
	rts

;
;	FIXME: optimise!
;
farcopy:
	; FIXME: - set up bank regs

copywl:
	ldaa tmp2
	staa porta
	ldaa ,x
	stab porta
	staa ,y
	inx
	iny
	dec tmp1
	bne copywl
	ldaa tmp3		; saved original bank
	staa porta
	rts

; FIXME: to do
farzcopy:
	rts


farcall:
	tpa
	psha
	sei		; interrupts off a minute
	staa tmp2	; save irq flags
	sts tmp1	; save stack
	jsr farcall2	; we need a helper on the return path
	; we come back on the new stack
	puly		; old sp
	pulb		; old bank
	pula		; flags (irq state)
	sei		; stack will be invalid for an instruction
	tys		; back to old stack
	stab porta	; and old bank
	tap		; irq flags back to entry state
	rts


farcall2:
	bsr bankcomp
	staa porta
	tys			; new stack now valid
	ldy tmp1		; old stack
	ldaa tmp2		; restore irq state
	tap
	psha
	pshb			; save old bank code on new stack
	pshy			; save old stack on new stack
	pshx			; target address
	ldy ret_y		; set up registers
	ldx ret_x
	ldd ret_d
	rts


farjump:
	tap
	sei
	staa tmp2
	bsr bankcomp
	staa porta
	tys			; new stack valid
	ldaa tmp2
	tap			; irq status back
	pshx
	ldy ret_y
	ldx ret_x
	ldd ret_d
	rts

reboot:
	sei
	jmp 0xF800

;
;	Make a cross domain IRQ handler call (X = descriptor)
;
;	We stack the registers on the current stack, then switch to bank 0
;	and invoke the handler (with interrupts still disabled) in bank 0.
;	Upon completion we switch back to the old stack and recover
;
interrupt:
	tsy			; save old stack frame
	ldab porta		; old bank
	tba
	andb #0x3F		; bank bits only
	anda #0xC0		; control bits
	staa porta		; now in kernel bank
	;
	;	Don't read the vectors until we switch bank. The vectors
	;	can then live in bank 0 space, not precious 
	;
	lds 2,x			; IRQ stack vector (set by OS)
	pshy			; save old stack
	pshb			; save bank bits
	ldx ,x			; vector is (func, stack)
	jsr ,x			; call
	pulb			; old bank
	puly			; old stack pointer
	ldaa porta
	anda #0xC0
	aba
	staa porta		; back to calling bank
	tys			; back to old stack
	rti

int_sci:
	ldx #vector_sci
	bra interrupt
int_spi:
	ldx #vector_spi
	bra interrupt
int_pai:
	ldx #vector_pai
	bra interrupt
int_pao:
	ldx #vector_pao
	bra interrupt
int_to:
	ldx #vector_to
	bra interrupt
int_tic4oc5:
	ldx #vector_toc5
	bra interrupt
int_toc4:
	ldx #vector_toc4
	bra interrupt
int_toc3:
	ldx #vector_toc3
	bra interrupt
int_toc2:
	ldx #vector_toc2
	bra interrupt
int_toc1:
	ldx #vector_toc1
	bra interrupt
int_tic3:
	ldx #vector_tic3
	bra interrupt
int_tic2:
	ldx #vector_tic2
	bra interrupt
int_tic1:
	ldx #vector_tic1
	bra interrupt
int_rti:
	ldx #vector_rti
	bra interrupt
int_irq:
	ldx #vector_irq
	bra interrupt
int_xirq:
	ldx #vector_xirq
	bra interrupt
int_swi:
	ldx #vector_swi
	bra interrupt
int_ill:
	ldx #vector_ill
	bra interrupt
int_cop:
	ldx #vector_cop
	bra interrupt
int_cmf:
	ldx #vector_cmf
	bra interrupt

boot1:	.ascii "68HC811 Bootware"
	.byte 13,10,0
boot2:	.byte 13,10
	.ascii "512Kb RAM"
	.byte 13,10,0
bootfail:
	.byte 13,10
	.ascii "No boot device found"
	.byte 13,10,0
spi_probe1:
	.byte 13,10
	.ascii "sd0: "
	.byte 0
spi_badcardv:
	.ascii "Invalid voltage"
	.byte 13,10,0
ioerror:
	.ascii "I/O error"
	.byte 13,10,0
boot_sdhc:
	.ascii "SDHC"
	.byte 0
boot_sdv1:
	.ascii "SD"
	.byte 0
boot_mmc:
	.ascii "MMC"
	.byte 0
notbootmedia:
	.ascii " - not bootable"
	.byte 13,10,0
sdbooting:
	.ascii " - bootable",
	.byte 13,10,13,10,0
hitnewline:
	.ascii "Hit newline to retry boot"
newline:
	.byte 13,10,0
sergo:
	.ascii "Serial boot: begin download"
	.byte 13,10
	.ascii ">"
	.byte 0

spi_read:
	rts
spi_write:
	rts
conin:
	cmpb #2			; no wait
	beq conin_nb
	cmpb #1
	beq conin_ne
	jsr waitkey
	jsr outchar
	rts
conin_ne:
	jmp waitkey
conin_nb:
	ldab scsr
	andb #0x20
	beq retff
	ldab scdr
	rts
retff:	ldab #0xff
	rts

conputs:
	jmp outstr
conputc:
	jmp outchar
conputhex:
	jmp outcharhex

block_read:
	ldab #0xff
	rts
block_write:
	ldab #0xff
	rts

	.sect .syscalls,"ax"		; starts at 0xFF40 for now

jfargetb:
	jmp fargetb
jfargetw:
	jmp fargetw
jfarputb:
	jmp farputb
jfarputw:
	jmp farputw
jfarcopy:
	jmp farcopy
jfarcall:
	jmp farcall
jfarjump:
	jmp farjump
jreboot:
	jmp reboot
jfarzcopy:
	jmp farzcopy
jfarzero:
	jmp farzero
jblockread:
	jmp block_read
jblockwrite:
	jmp block_write
jconputs:
	jmp conputs
jconputc:
	jmp conputc
jconputhex:
	jmp conputhex
jconin:
	jmp conin
jspiread:
	jmp spi_read
jspiwrite:
	jmp spi_write

	.sect .vectors		; FIXME: set to 0xFFD6 in the link file

	.word	int_sci
	.word	int_spi
	.word	int_pai
	.word	int_pao
	.word	int_to
	.word	int_tic4oc5
	.word	int_toc4
	.word	int_toc3
	.word	int_toc2
	.word	int_toc1
	.word	int_tic3
	.word	int_tic2
	.word	int_tic1
	.word	int_rti
	.word	int_irq
	.word	int_xirq
	.word	int_swi
	.word	int_ill
	.word	int_cop
	.word	int_cmf
	.word	_start		; Reset vector
