;
;	The machine boots in 6502 mode. We are loaded as 3584 byte block
;	at 0x0300.
;
	.6502		; Classic CPU only

	.abs

	; We get no choice. We can specify a load address but
	; many ROMs then jump to 2200 regardless of where it loaded!
	.org 0x2200

start:
	lda	#0xC3
	sta	@0
	lda	#<go
	sta	@1
	lda	#>go
	sta	@2
	; now contains a Z80 jump to the first Z80 bytes
	; Now say hello
	ldy	#0
next:
	lda	0xFC00
	and	#2
	beq	next
	lda	hello,y
	beq	done_hello
	sta	0xFC01
	iny
	; Will be NE
	bne	next
done_hello:
	; Now perform the rites of passage
	lda	#0x04		; Data register
	sta	0xF701
	lda	#0xFF
	sta	0xF700
	lda	#0x00		; Direction register
	sta	0xF701
	lda	#0xFF		; Output
	sta	0xF700
	lda	#0x04
	sta	0xF701
	lda	#0x5F
	sta	0xF700
	; never happens. We reset here into Z80, execute the JP at 0
wait:	bne	wait
hello:
	.byte	#0x1B,#0x1C
	.ascii	"Challenger III Fuzix Loader v 0.1"
	.byte	#13,#10,#13,#10,#0

	;	Rest of logical sector free, start Z80 code after it

	.org	0x2400
go:
