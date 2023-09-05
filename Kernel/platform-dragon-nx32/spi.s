;
;	SPI interface for the 65SPI on the NX32 with SPI
;

	.module dragonspi

	.globl _spi_setup
	.globl _sd_spi_clock
	.globl _sd_spi_lower_cs
	.globl _sd_spi_raise_cs
	.globl _sd_spi_transmit_byte
	.globl _sd_spi_receive_byte
	.globl _sd_spi_receive_sector
	.globl _sd_spi_transmit_sector

	.globl _blk_op

        include "kernel.def"
        include "../kernel09.def"

; 65SPI control flags
FRX	equ 0x10		; Fast Receive mode
ECE	equ 0x04		; External Clock Enable

	.area .text

_spi_setup:
	clra
	sta SPICTRL		; just in case
	lda #0x0F
	sta SPISIE		; selects off, IRQs off
	cmpa SPISIE
	bne nospi
	lda SPIDATA		; clear TC
	lda SPISTATUS
	bmi nospi 		; TC not clear -> no spi present
	sta SPIDATA		; start a transmit, TC should now be clear
	lda SPISTATUS
	bpl spigood
nospi:	clrb
	rts
spigood:
	ldb #1
	rts


;
;	May need to switch clocks too
;
_sd_spi_clock:
	cmpb #0			
	beq slow
	ldd #ECE*256+0x01	; external 45MHz clock on, divide by 4
	bra clkset
slow:	ldd #0x0000		; internal clock, phi/2 -> 0.89MHz/2 = 445kHz
clkset:	std SPICTRL
	rts

;
;	For multiple cards these need to look at the card #
;
_sd_spi_raise_cs:
	lda SPISIE
	ora #SPICS
	sta SPISIE
	rts

_sd_spi_lower_cs:
	lda SPISIE
	anda #0xFF-SPICS
	sta SPISIE
	rts

;
;	We must check for busy as these methods are used at low speed
;
_sd_spi_transmit_byte:
	stb SPIDATA
txwait:
	lda SPISTATUS
	anda #0x20		; BSY
	bne txwait
	rts

_sd_spi_receive_byte:
	lda #0xFF
	sta SPIDATA
rxwait:
	lda SPISTATUS		; wait for bit 7 to set
	bpl rxwait
	ldb SPIDATA
	rts

	.area .common

;
;	Swap not done yet.
;
;	The SPI clock must be in fast mode here as we don't poll the busy
;	bits but know that by the time the CPU arrives at the next byte
;	the transaction is done. Note - we don't have to worry about the
;	interrupt situation here, we drive the clocks so if we go off for
;	an interrupt all is fine.
;
_sd_spi_receive_sector:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	ldx _blk_op
	leay 512,x
	sty endspi
	lda _blk_op+2
	beq rdspi
	jsr map_proc_always
rdspi:	lda #ECE+FRX		; FRX on, external clock on
	sta <SPICTRL
	lda <SPIDATA		; read old value, triggers shifting in new
read8:
	lda <SPIDATA
	ldb <SPIDATA
	std ,x++
	lda <SPIDATA
	ldb <SPIDATA
	std ,x++
	lda <SPIDATA
	ldb <SPIDATA
	std ,x++
	lda <SPIDATA
	ldb <SPIDATA
	std ,x++
	cmpx endspi
	bne read8
	jsr map_kernel
	lda #ECE
	sta <SPICTRL		; FRX off, external clock on
	puls y,dp,pc

_sd_spi_transmit_sector:
	pshs y,dp
	lda #0xFF
	tfr a,dp
	ldx _blk_op
	leay 512,x
	sty endspi
	lda _blk_op+2
	beq wrspi
	jsr map_proc_always
wrspi:	lda #ECE		; ext clock, no FRX
	sta SPICTRL
write8:
	ldd ,x++
	sta <SPIDATA
	stb <SPIDATA
	ldd ,x++
	sta <SPIDATA
	stb <SPIDATA
	ldd ,x++
	sta <SPIDATA
	stb <SPIDATA
	ldd ,x++
	sta <SPIDATA
	stb <SPIDATA
	cmpx endspi
	bne write8
	jsr map_kernel
	puls y,dp,pc

	.area .commondata

endspi:	.dw 0
