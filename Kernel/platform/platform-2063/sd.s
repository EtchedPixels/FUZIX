;
;	SD card interface
;
;	This is absolutely foul because the design puts the page registers
;	and the bitbanging on the same port. We can't have interrupt code
;	doing bank switching during an I/O or we get subtle corruption.
;
;	As blocking interrupts would suck for serial we instead keep a
;	busy flag for the gpio and defer timer activity (which does bank
;	switch) but not serial which does not.
;

	.module	sd

	.area	_COMMONMEM


	.globl	_sd_spi_slow
	.globl	_sd_spi_fast
	.globl	_sd_spi_raise_cs
	.globl	_sd_spi_lower_cs
	.globl	_sd_spi_rx_byte
	.globl	_sd_spi_tx_byte
	.globl	_sd_spi_rx_sector
	.globl	_sd_spi_tx_sector

	.globl	_gpio
	.globl	_td_raw
	.globl	_td_page

	.globl  _sd_busy

	.globl	map_for_swap
	.globl	map_kernel
	.globl	map_proc_always

_sd_spi_slow:
_sd_spi_fast:
	ret

_sd_spi_lower_cs:
	ld	a,#1
	ld	(_sd_busy),a
	ld	a,(_gpio)
	and	#0xFD
	or	#0x01
	out	(0x10),a
	and	#0xFB
	ld	(_gpio),a
	out	(0x10),a
sd_spi_idle:
	xor	a
	ld	(_sd_busy),a
	ret
_sd_spi_raise_cs:
	ld	a,#1
	ld	(_sd_busy),a
	ld	a,(_gpio)
	and	#0xFD
	out	(0x10),a
	or	#0x05
	ld	(_gpio),a
	out	(0x10),a
	; Fall through
_sd_spi_rx_byte:
	ld	a,#1
	ld	(_sd_busy),a
	ld	a,(_gpio)
	or	#0x01		; data high
	and	#0xFD		; clock
	ld	d,a
	add	#2
	ld	e,a		; D is clock low E is high
	ld	c,#0x10
	call	sd_rx
	jr	sd_spi_idle

	; Entry point with registers set up
sd_rx:
	;	Clock the bits	(47 clocks a bit) or about 25K/second minus
	; 	overheads per byte - so nearer 20K which is adequate for our
	; 	needs
	;	Bit 0
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 1
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 2
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 3
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 4
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 5
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 6
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	;	Bit 7
	out	(c),d
	out	(c),e
	in	a,(0x00)
	rla	
	rl	l
	ret

;
;	Slightly less performance critical which
;	is good as it's annoying on this setup
;	due to the shared gpio
;
;	Byte to send is in L
;
_sd_spi_tx_byte:
	ld	a,#1
	ld	(_sd_busy),a

	ld	a,(_gpio)
	and	#0xFD		; clock high
	;	Clock the bits out. Ignore reply
	;	48 clocks a bit
	;	
	ld	h,#2	; saves us 6 clocks a bit
	;
	;	Bit 0
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 1
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 2
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 3
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 4
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 5
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 6
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a
	sub	h
	;	Bit 7
	rra
	rl	l
	rla
	out	(0x10),a
	add	h
	out	(0x10),a

	xor	a
	ld	(_sd_busy),a
	ret

_sd_spi_rx_sector:
	push	ix
	push	hl
	pop	ix
	ld	bc,#0xFF		; 0 for count 255 for reload of A
	ld	a,(_td_raw)
	or	a
	jr	z, rx_byte
	dec	a
	jr	z, rx_user
	ld	a,(_td_page)
	call	map_for_swap
	jr	rx_byte
rx_user:
	call	map_proc_always
rx_byte:
	call	_sd_spi_rx_byte
	;	Mark the SD busy (sd_spi_rx_byte marked it idle
	;	and keep it busy while we burst the block. It's a trade
	;	off versus slower I/O
	ld	a,#1
	ld	(_sd_busy),a
	ld	(ix),l
	inc	ix
	call	sd_rx
	ld	(ix),l
	inc	ix
	ld	b,#0xFF		; 510 bytes
rx_loop:
	call	sd_rx
	ld	(ix),l	
	inc	ix
	call	sd_rx
	ld	(ix),l
	inc	ix
	djnz	rx_loop
	pop	ix
	xor	a
	ld	(_sd_busy),a
	jp	map_kernel

_sd_spi_tx_sector:
	push	ix
	push	hl
	pop	ix
	ld	b,#0
	ld	a,(_td_raw)
	or	a
	jr	z, tx_byte
	dec	a
	jr	z, tx_user
	ld	a,(_td_page)
	call	map_for_swap
	jr	tx_byte
tx_user:
	call	map_proc_always
tx_byte:
	ld	l,(ix)
	inc	ix
	call	_sd_spi_tx_byte
	ld	l,(ix)
	inc	ix
	call	_sd_spi_tx_byte
	djnz	tx_byte
	pop	ix
	jp	map_kernel
