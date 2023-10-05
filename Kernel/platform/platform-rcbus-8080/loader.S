#define pchar		0x08
#define print		0x0B
#define ide_readb	0x0E
#define ide_read_data	0x11
#define ide_writeb	0x14
#define ide_waitdrq	0x17
#define ide_waitready	0x1A
#define pready	        0x1D
#define iready	        0x20
#define ichar           0x23

;
;	A loader bootblock to run at 0xFE00
;
;	This version for the 8085/MMU card
;

	.code
;
;	We stick it all in text
;
	.word 0x8085

start:
	lxi h,Ok
	call print

	lxi h,0x0100
	lxi sp,0xFB00

	;	HL	= load address
	;	D	= counts bytes for sector loads
	;	E	= current sector
	;
	xra a
dread:
	inr a
	cpi 125
	jz load_done
	push h
	mov e,a
	mov l,a
	mvi a,0x0B
	call ide_writeb
	mvi l,1
	mvi a,0x0A
	call ide_writeb
	mvi a,0x0F
	mvi l,0x20
	call ide_writeb

	mvi a,'.'
	call pchar

	call ide_waitdrq
	lxi h,0xFB00	; bounce buffer
	push d
	call ide_read_data
	call ide_waitready
	pop d
	pop h
	mvi a,3
	out 0xFF
	call bounce
	mvi a,1
	out 0xFF
	mov a,e
	jmp dread

load_done:
	mvi a,3
	out 0xFF	; Go to all RAM
	lda 0x0100
	cpi 0x80
	jnz bad_load
	lda 0x0101
	cpi 0x80
	jnz bad_load
	mvi a,1
	out 0xFF
	lxi h,running
	call print
	mvi a,3
	out 0xFF
	jmp 0x0102	; Run the loaded OS image

bad_load:
	mvi a,1
	out 0xFF	; ROM back in
	lxi h,badimg
	call print
	hlt

;
;	Copy 512 bytes from 0xFB00 to H leaving H adjusted to the next
;	block. Preserves BCDE
;
bounce:
	push d
	push b
	lxi d,0xFB00
	mvi b,0
bounceloop:
	ldax d
	mov m,a
	inx d
	inx h
	ldax d
	mov m,a
	inx d
	inx h
	dcr b
	jnz bounceloop
	pop b
	pop d
	ret

	.data

Ok:
	.ascii 'Ok'
	.byte 13,10,0

badimg:
	.byte 13,10
	.ascii 'Image not bootable.'
	.byte 13,10,0

running:
	.byte 13,10
	.ascii 'Boot complete.'
	.byte 13,10,0
