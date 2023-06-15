#include "../kernel-8080.def"
;
;	First draft on an 8085 PPIDE driver.
;
;	Does not yet include swap support.
;

#define PPIDE_PPI_BUS_READ	0x92
#define PPIDE_PPI_BUS_WRITE	0x80

#define PPIDE_PPI_WR_LINE	0x20
#define PPIDE_PPI_RD_LINE	0x40

#define IDE_REG_STATUS		0x0F
#define IDE_REG_DATA		0x08

#define BLKPARAM_ADDR_OFFSET	0
#define BLKPARAM_IS_USER_OFFSET	2

	.setcpu 8080

	.code

	.export _devide_readb
;
;	Read a byte from an IDE register and hand it back to the C code
;
_devide_readb:
	lxi	h,2
	dad	sp
	mov	a,m
	out	0x22
	mov	d,a
	ori	PPIDE_PPI_RD_LINE
	out	0x22
	in	0x20
	mov	l,a
	mov	a,d
	out	0x22
	mvi	h,0
	ret

	.export _devide_writeb
;
;	Write a byte to an IDE register for C code
;
_devide_writeb:
	lxi	h,2		; regaddr
	dad	sp
	mov	e,m
	inx	h
	mov	d,m
	inx	h		; value
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	
	mvi	a,PPIDE_PPI_BUS_WRITE
	out	0x23
	mov	a,e		; Register
	out	0x22
	mov	a,l		; 16bits of value
	out	0x20
	mov	a,h
	out	0x21
	mov	a,e		; Waggle WR line
	ori	PPIDE_PPI_WR_LINE
	out	0x22
	mov	a, e
	out	0x22
	mvi	a,PPIDE_PPI_BUS_READ
	out	0x23		; Bus direction back
	ret

	.common

	.export _devide_read_data
;
;	Perform the data block transfer when reading. blk_op holds all the
;	parameter information we need
;
_devide_read_data:
	push	b
	; Point at the data register
	mvi	a,IDE_REG_DATA
	out	0x22
	; Get the destination buffer
	lhld	_blk_op + BLKPARAM_ADDR_OFFSET
	; Set d and e up as fast copies of the values we need to out
	mov	d,a
	ori	PPIDE_PPI_RD_LINE
	mov	e,a
	; If we are doing a user transfer map the process
	; We don't yet support swap or buffer mappings (FIXME)
	lda	_blk_op + BLKPARAM_IS_USER_OFFSET
	ora	a
	push	psw
	; TODO: SWAP
	cnz	map_proc_always
	; 256 words per transfer
	mvi	b,0
goread:
	; Write the register | RD
	mov	a,e
	out	0x22
	; Read the byte back from the interface (no polling - it is faster
	; than we are)
	in	0x20
	; Save it
	mov	m,a
	inx	h
	; Repeat for second byte
	in	0x21
	mov	m,a
	inx	h
	; Drop RD
	mov	a,d
	out	0x22
	; Check if done
	dcr	b
	jnz	goread
	; Fix up mappings
	pop	psw
	pop	b
	rz
	jmp	map_kernel

	.export _devide_write_data

_devide_write_data:
	; Point at the data register
	mvi	a,IDE_REG_DATA
	out	0x22
	; Switch the bus direction on the 82C55 for ports A and B
	mvi	a,PPIDE_PPI_BUS_WRITE
	out	0x23
	; Get the source buffer
	lhld	_blk_op + BLKPARAM_ADDR_OFFSET
	; Set up d and e as the register values we need to out
	mvi	d,IDE_REG_DATA
	mvi	e,IDE_REG_DATA + PPIDE_PPI_WR_LINE
	; Map the user space if needed
	; Need to do swap and buffer mappings properly
	lda	_blk_op + BLKPARAM_IS_USER_OFFSET
	ora	a
	push	b
	push	psw
	cnz	map_proc_always
	mvi	b,0
gowrite:
	; Write bit off
	mov	a,d
	out	0x22
	; Now put the data value on the bus (the register is already set)
	mov	a,m
	inx	h
	out	0x20
	mov	a,m
	inx	h
	out	0x21
	; Strobe write with data pending and settled
	mov	a,e
	out	0x22
	; Check if we are done
	dcr	b
	jnz	gowrite
	; Drop the write bit
	mov	a,d
	out	0x22
	; Put the bus back in the read direction as all other code expects
	mvi	a,PPIDE_PPI_BUS_READ
	out	0x23
	; Recover the mappings
	pop	psw
	pop	b
	rz
	jmp	map_kernel

	.discard

	.export _ppide_init

;
;	Initialize PPIDE. This consists of setting up the registers
;	for read state
;
_ppide_init:
	mvi	a,PPIDE_PPI_BUS_READ
	out	0x23
	mvi	a,IDE_REG_STATUS
	out	0x22
	ret
