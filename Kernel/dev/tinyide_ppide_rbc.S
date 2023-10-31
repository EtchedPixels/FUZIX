#include "../build/kernel.def"

#ifdef CONFIG_TINYIDE_PPI

	.code

	.export _ppide_init
	.export _ide_read
	.export _ide_write
	.export _devide_read_data
	.export _devide_write_data

_ppide_init:
	ld	a,PPIDE_PPI_BUS_READ
	out	(ppi_port_c),a
	ret

_ide_read:
	pop	de
	pop	hl
	push	hl	
	push	de
	ld	a,l
	out	(ppi_port_c),a
	or	#PPIDE_RD_LINE
	out	(ppi_port_c),a
	in	a,(ppi_port_a)
	ld 	e,a
	ld 	a,l
	out	(ppi_port_c),a
	ld	l,e
	ld	h,0
	ret

_ide_write:
	ld	hl,4
	add	hl,sp
	ld	e,(hl)
	dec	hl
	dec	hl
	ld	d,(hl)

	ld	a,PPIDE_PPI_BUS_WRITE
	out	(ppi_control),a
	ld	a,e
	out	(ppi_port_a),a
	xor	a
	out	(ppi_port_b),a
	ld	a,d
	or	#PPIDE_WR_LINE
	out	(ppi_port_c),a
	ld	a,d
	out	(ppi_port_c),a
	ld	a,PPIDE_PPI_BUS_READ
	out	(ppi_control),a
	ret
	

devide_map:
	ld	a, (_td_raw)
	or	a
	jp	z, map_buffers
	dec	a
	jp	z, map_proc_always
	ld	a,(_td_page)
	jp	map_for_swap

_devide_read_data:
#ifdef CONFIG_BANKED
#error "not supported yet"
#else
	pop	de
	pop	hl
	push	hl
	push	de
#endif
	push	bc
	ld	a, ppide_data
	ld	c,ppi_port_c		; select control lines
	out	(c), a			; select IDE data register
	ld	d, ppide_data		; register address
	ld	e, ppide_data + PPIDE_RD_LINE	; register address with /RD asserted
	ld	b, 0			; setup count
	call	devide_map
	ld	a, ppi_port_a		; I will be needing this later
goread:					; now we do the transfer
	out	(c), e			; assert /RD
	ld	c, a			; PPIDE_BASE
	ini				; read byte from LSB
	inc	c			; up to MSB
	ini				; read byte from MSB
	inc	c			; control lines
	out	(c), d			; de-assert /RD
	inc	b			; (delay) counteract second ini instruction
	jr	nz, goread		; (delay) next word
	; read completed
	pop	bc
	jp	map_kernel		; map kernel then return

_devide_write_data:
#ifdef CONFIG_BANKED
#error "unsupported"
#else
	pop	de
	pop	hl
	push	hl
	push	de
#endif
	push	bc
	ld	c, ppi_port_c		; select control lines
	ld	a, ppide_data
	out	(c), a			; select data register
	ld	a, PPIDE_PPI_BUS_WRITE
	inc	c			; up to 8255A command register
	out	(c), a			; 8255A ports A, B to output mode
	dec	c			; back down to the control lines
	ld	d, ppide_data		; register address
	ld	e, ppide_data + PPIDE_WR_LINE	; register address with /WR asserted
	ld	b, 0			; setup count
	call	devide_map
	ld	a, ppi_port_a		; I will be needing this later
gowrite:				; now we do the transfer
	out (c), d			; de-assert /WR
	ld	c, a			; PPIDE_BASE
	outi				; write byte to LSB
	inc	c			; up to MSB
	outi				; write byte to MSB
	inc	c			; up to control lines
	out	(c), e			; assert /WR
	inc	b			; (delay) offset to counteract second outi instruction
	jr	nz, gowrite		; (delay) next word
	; write completed
	out	(c), d			; de-assert /WR
	ld	a, PPIDE_PPI_BUS_READ
	inc	c			; up to 8255A command register
	out	(c), a			; 8255A ports A, B to read mode
	pop	bc
	jp	map_kernel		; map kernel then return

#endif
