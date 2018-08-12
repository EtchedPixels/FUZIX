;
;	Bootstrap for cartridge start up. For now load from block 1 (second
;	block). We can refine this later. We do 51 block loads from block 1
;	for 1A00 to 7FFF and then copy 1A00 to 0100 for vectors
;
;	Needs some kind of global timeout -> error handling
;
;
;	Load block b
;
	.module bootstrap

	.globl load_image
	.globl _cocoswap_dev

	.area .text

_cocoswap_dev:
	.dw 0

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
	ldy #$0400			; display at this point
	lda #'G'
	sta ,y+
	ldb #1
	lda #51
	ldx #$1A00
load_loop:
	pshs a,b
	bsr load_block
	lda #'*'
	sta ,y+
	puls a,b
	incb
	deca
	bne load_loop
	ldx #$1A00
	ldu #$0100
vec_copy:
	ldd ,x++
	std ,u++
	cmpu #$0200
	bne vec_copy

	ldx #$1A00
	clra
	clrb
ud_wipe:
	std ,x++
	cmpx #$1C00
	bne ud_wipe
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
