
	include "../../build/kernel.def"
	include "../../cpu-6809/kernel09.def"

	.area .common

	.globl _sddrive_receive_sector
	.globl _sddrive_transmit_sector
	.globl _sddrive_transmit_byte
	.globl _sddrive_receive_byte

;
;	SDDrive. The fastest bit bang option. Has hardware generating
;	the SPI clock pulses. Note this will not work with DCMOTO as
;	the emulator doesn't emulate SD properly and even if you avoid
;	commands it doesn't like then doesn't present the actual data but
;	makes up two sectors in front of it.
;
sddrive_readb:
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	cmpb	<$BF
	rola
	rts

_sddrive_receive_byte:
	pshs	dp
	ldb	#IOPAGE
	tfr	b,dp
	ldb	#$7F
	bsr	sddrive_readb
	tfr	a,b
	tfr	d,x
	puls	dp,pc
	
sddrive_writeb:
	rola
	rola
	sta <$BF
	rola
	sta <$BF
	rola
	sta <$BF
	rola
	sta <$BF
	rola
	sta <$BF
	rola
	sta <$BF
	rola
	sta <$BF
	rola
	sta <$BF
	rts

_sddrive_transmit_byte:
	pshs	dp
	; byte in B
	tfr	b,a
	ldb	#IOPAGE
	tfr	b,dp
	bsr	sddrive_writeb
	puls	dp,pc

_sddrive_receive_sector:
	pshs	y,dp
	lda	#IOPAGE
	tfr	a,dp
	; X is the data pointer
	lda	_td_raw
	beq	sdd_rx_kernel
	deca
	beq	sdd_rx_user
	ldb	_td_page
	jsr	map_for_swap
sdd_rx_kernel:
	ldb	#$7F
	ldy	#32		; 32 x 16 bytes (512 byte sector)
sdd_rx_loop:
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	jsr	sddrive_readb
	sta	,x+
	; 16 byte a loop
	leay	-1,y
	bne	sdd_rx_loop
	puls	y,dp
	jmp	map_kernel
sdd_rx_user:
	jsr	map_proc_always
	bra	sdd_rx_kernel

_sddrive_transmit_sector:
	pshs	dp
	lda	#IOPAGE
	tfr	a,dp
	; X is the data pointer
	lda	_td_raw
	beq	sdd_tx_kernel
	deca
	beq	sdd_tx_user
	ldb	_td_page
	jsr	map_for_swap
sdd_tx_kernel:
	ldb	#32		; 32 x 16 bytes (512 byte sector)
sdd_tx_loop:
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	lda	,x+
	jsr	sddrive_writeb
	; 16 byte a loop
	decb
	bne	sdd_tx_loop
	puls	dp
	jmp	map_kernel
sdd_tx_user:
	jsr	map_proc_always
	bra	sdd_tx_kernel

;
;	SDMO - first generation interface that uses the
;	tape port for communcations (the lightpen port is
;	only used to take a 5v supplyy)
;
	.globl _sdmo_receive_sector
	.globl _sdmo_transmit_sector
	.globl _sdmo_transmit_byte
	.globl _sdmo_receive_byte

sdmo_readb:
	ldb	#$FE
sdm_rb:
	lda	#$7F
	sta	<$C2
	cmpa	<$C0
	lda	#$36
	sta	<$C2
	rolb
	bcs	sdm_rb
	tfr	d,x
	rts

_sdmo_receive_byte:
	pshs	dp
	lda	#IOPAGE
	tfr	a,dp
	bsr	sdmo_readb
	; byte in b
	puls	dp,pc
	
sdmo_writeb:
	ldb	#8
	pshs	b
sdm_wb:
	ldb	<$C0
	orb	#$40
	dec	,s
	bmi	sdmo_wbdone
	asla
	bcs	sdmo_w1
	andb	#$8F
sdmo_w1:
	stb	<$C0
	ldb	#$3E
	stb	<$C2
	bra	sdm_wb
sdmo_wbdone:
	puls	b,pc

_sdmo_transmit_byte:
	pshs	dp
	lda	#IOPAGE
	tfr	a,dp
	bsr	sdmo_writeb
	; byte in b
	puls	dp,pc

	
_sdmo_receive_sector:
	pshs	y,dp
	lda	#IOPAGE
	tfr	a,dp
	; X is the data pointer
	lda	_td_raw
	beq	sdmo_rx_kernel
	deca
	beq	sdmo_rx_user
	ldb	_td_page
	jsr	map_for_swap
sdmo_rx_kernel:
	ldb	#$7F
	ldy	#32		; 32 x 16 bytes (512 byte sector)
sdmo_rx_loop:
	jsr	sdmo_readb	;	1
	sta	,x+
	jsr	sdmo_readb	;	2
	sta	,x+
	jsr	sdmo_readb	;	3
	sta	,x+
	jsr	sdmo_readb	;	4
	sta	,x+
	jsr	sdmo_readb	;	5
	sta	,x+
	jsr	sdmo_readb	;	6
	sta	,x+
	jsr	sdmo_readb	;	7
	sta	,x+
	jsr	sdmo_readb	;	8
	sta	,x+
	jsr	sdmo_readb	;	9
	sta	,x+
	jsr	sdmo_readb	;	10
	sta	,x+
	jsr	sdmo_readb	;	11
	sta	,x+
	jsr	sdmo_readb	;	12
	sta	,x+
	jsr	sdmo_readb	;	13
	sta	,x+
	jsr	sdmo_readb	;	14
	sta	,x+
	jsr	sdmo_readb	;	15
	sta	,x+
	jsr	sdmo_readb	;	16
	sta	,x+
	; 16 byte a loop
	leay	-1,y
	bne	sdmo_rx_loop
	puls	y,dp
	jmp	map_kernel
sdmo_rx_user:
	jsr	map_proc_always
	bra	sdmo_rx_kernel

_sdmo_transmit_sector:
	pshs	y,dp
	lda	#IOPAGE
	tfr	a,dp
	; X is the data pointer
	lda	_td_raw
	beq	sdmo_tx_kernel
	deca
	beq	sdmo_tx_user
	ldb	_td_page
	jsr	map_for_swap
sdmo_tx_kernel:
	ldb	#32		; 32 x 16 bytes (512 byte sector)
sdmo_tx_loop:
	lda	,x+
	jsr	sdmo_writeb	;	1
	lda	,x+
	jsr	sdmo_writeb	;	2
	lda	,x+
	jsr	sdmo_writeb	;	3
	lda	,x+
	jsr	sdmo_writeb	;	4
	lda	,x+
	jsr	sdmo_writeb	;	5
	lda	,x+
	jsr	sdmo_writeb	;	6
	lda	,x+
	jsr	sdmo_writeb	;	7
	lda	,x+
	jsr	sdmo_writeb	;	8
	lda	,x+
	jsr	sdmo_writeb	;	9
	lda	,x+
	jsr	sdmo_writeb	;	10
	lda	,x+
	jsr	sdmo_writeb	;	11
	lda	,x+
	jsr	sdmo_writeb	;	12
	lda	,x+
	jsr	sdmo_writeb	;	13
	lda	,x+
	jsr	sdmo_writeb	;	14
	lda	,x+
	jsr	sdmo_writeb	;	15
	lda	,x+
	jsr	sdmo_writeb	;	16
	; 16 byte a loop
	decb
	bne	sdmo_tx_loop
	puls	y,dp
	jmp	map_kernel
sdmo_tx_user:
	jsr	map_proc_always
	bra	sdmo_tx_kernel

;
;	SDMoto - second generation of the SD reader that works by
;	bitbanging the PIA controlled joystick having changed
;	the direction of some bits.
;
	.globl _sdmoto_receive_sector
	.globl _sdmoto_transmit_sector
	.globl _sdmoto_transmit_byte
	.globl _sdmoto_receive_byte
	.globl _sdmoto_init

sdmoto_readb:
	ldb	#$FE
sdmt_rb:
	lda	#$7F
	sta	<$CC
	cmpa	$CC
	lda	$5F
	sta	<$CC
	rolb
	bcs	sdmt_rb
	rts

_sdmoto_receive_byte:
	pshs	dp
	lda	#IOPAGE
	tfr	a,dp
	bsr	sdmoto_readb
	; byte in b
	tfr	d,x
	puls	dp,pc
	
sdmoto_writeb:
	ldb	#8
	pshs	b
sdmt_wb:
	asla
	rorb
	lsrb
	stb	<$CC
	andb	#$DF
	stb	<$CC
	dec	,s
	bne	sdmt_wb
	orb	#$40
	stb	<$CC
	puls	b,pc

_sdmoto_transmit_byte:
	pshs	dp
	lda	#IOPAGE
	tfr	a,dp
	bsr	sdmoto_writeb
	; byte in b
	puls	dp,pc
	
	
_sdmoto_receive_sector:
	pshs	y,dp
	lda	#IOPAGE
	tfr	a,dp
	; X is the data pointer
	lda	_td_raw
	beq	sdmt_rx_kernel
	deca
	beq	sdmt_rx_user
	ldb	_td_page
	jsr	map_for_swap
sdmt_rx_kernel:
	ldb	#$7F
	ldy	#32		; 32 x 16 bytes (512 byte sector)
sdmt_rx_loop:
	jsr	sdmoto_readb	;	1
	sta	,x+
	jsr	sdmoto_readb	;	2
	sta	,x+
	jsr	sdmoto_readb	;	3
	sta	,x+
	jsr	sdmoto_readb	;	4
	sta	,x+
	jsr	sdmoto_readb	;	5
	sta	,x+
	jsr	sdmoto_readb	;	6
	sta	,x+
	jsr	sdmoto_readb	;	7
	sta	,x+
	jsr	sdmoto_readb	;	8
	sta	,x+
	jsr	sdmoto_readb	;	9
	sta	,x+
	jsr	sdmoto_readb	;	10
	sta	,x+
	jsr	sdmoto_readb	;	11
	sta	,x+
	jsr	sdmoto_readb	;	12
	sta	,x+
	jsr	sdmoto_readb	;	13
	sta	,x+
	jsr	sdmoto_readb	;	14
	sta	,x+
	jsr	sdmoto_readb	;	15
	sta	,x+
	jsr	sdmoto_readb	;	16
	sta	,x+
	; 16 byte a loop
	leay	-1,y
	bne	sdmt_rx_loop
	puls	y,dp
	jmp	map_kernel
sdmt_rx_user:
	jsr	map_proc_always
	bra	sdmt_rx_kernel

_sdmoto_transmit_sector:
	pshs	y,dp
	lda	#IOPAGE
	tfr	a,dp
	; X is the data pointer
	lda	_td_raw
	beq	sdmt_tx_kernel
	deca
	beq	sdmt_tx_user
	ldb	_td_page
	jsr	map_for_swap
sdmt_tx_kernel:
	ldb	#32		; 32 x 16 bytes (512 byte sector)
sdmt_tx_loop:
	lda	,x+
	jsr	sdmoto_writeb	;	1
	lda	,x+
	jsr	sdmoto_writeb	;	2
	lda	,x+
	jsr	sdmoto_writeb	;	3
	lda	,x+
	jsr	sdmoto_writeb	;	4
	lda	,x+
	jsr	sdmoto_writeb	;	5
	lda	,x+
	jsr	sdmoto_writeb	;	6
	lda	,x+
	jsr	sdmoto_writeb	;	7
	lda	,x+
	jsr	sdmoto_writeb	;	8
	lda	,x+
	jsr	sdmoto_writeb	;	9
	lda	,x+
	jsr	sdmoto_writeb	;	10
	lda	,x+
	jsr	sdmoto_writeb	;	11
	lda	,x+
	jsr	sdmoto_writeb	;	12
	lda	,x+
	jsr	sdmoto_writeb	;	13
	lda	,x+
	jsr	sdmoto_writeb	;	14
	lda	,x+
	jsr	sdmoto_writeb	;	15
	lda	,x+
	jsr	sdmoto_writeb	;	16
	; 16 byte a loop
	decb
	bne	sdmt_tx_loop
	puls	y,dp
	jmp	map_kernel
sdmt_tx_user:
	jsr	map_proc_always
	bra	sdmt_tx_kernel

_sdmoto_init:
	pshs	dp
	lda	#IOPAGE
	tfr	a,dp
	lda	<$CE
	anda	#$FB
	sta	<$CE
	ldb	#$60
	stb	<$CC
	ora	#$04
	sta	<$CE
	puls	dp,pc
