;
;	Bootstrap for cartridge start up. For now load from block 1 (second
;	block). We can refine this later. We do 62 block loads from block 1
;	for 0400 to 7FFF
;
;	Needs some kind of global timeout -> error handling
;
;
;	Load block b
;
	.module bootstrap

	.globl load_image

	.area .text

load_block:
	lda #$FF
	tfr a,dp
load_block_1:
	lda #$80
	bita <$FF57			; busy
	bne load_block_1
	lda #$40
	sta <$FF56
load_block_2:
	lda #$80
	bita <$FF57			; busy
	bne load_block_2
	clr <$FF55
	clr <$FF54
	stb <$FF53			; block number to fetch
	lda #1
	sta <$FF52
	
	lda #$40
wait_drdy:
	bita <$FF57
	beq wait_drdy
	lda #$20			; read command
	sta <$FF57
	clrb
	lda #$08
wait_drq:
	bita <$FF57			; DRQ
	beq wait_drq
copyloop:
	lda <$FF50			; read word and latch
	sta ,x+
	lda <$FF58			; latched byte
	sta ,x+
	decb
	bne copyloop
	lda #$80
wait_bsy:
	bita <$FF57
	bne wait_bsy
	lda #$01
	bita <$FF57
	bne load_error
	rts


load_image:
	orcc #$50
	; COCO2 defaults to text at 0400-05FF move it to 0200
	ldx #0xFFC7
	clr ,x+			; Set F0
	clr ,x++		; Clear F1-F6
	clr ,x++
	clr ,x++
	clr ,x++
	clr ,x++
	clr ,x++
	ldx #$0200			; display at this point
	ldd #$6060
wiper:
	std ,x++
	cmpx #$0400
	bne wiper
	ldy #$0200
	lda #'G'
	sta ,y+
	ldb #1
	lda #62				; 31K straight load
	ldx #0x0400			; directly after video
load_loop:
	pshs a,b
	bsr load_block
	lda #'*'
	sta ,y+
	puls a,b
	incb
	deca
	bne load_loop

	ldx #0x0400			; check signature
	lda ,x+
	cmpa #$15
	bne wrong_err
	lda ,x+
	cmpa #$C0
	bne wrong_err
	rts

wrong_err:
	ldu #wrong
	bra l2
load_error:
	ldu #fail
l2:	bsr copy_str
l1:	bra l1

copy_str:
	lda ,u+
	beq endstr
	sta ,y+
	bra copy_str
endstr:	rts
fail:
	.ascii "FAIL"
	.db 0
wrong:
	.ascii "WRONG IMAGE"
	.db 0
