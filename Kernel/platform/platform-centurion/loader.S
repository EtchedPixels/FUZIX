;
;	Test loader for the Hawk drive
;
;	Mux is initialized, 5600 byte loaded from 0x0100
;
		.code
		.setcpu 6

;
;	Q: What is the true sectors per track 14 or 16 ?
;	Assume 14 for now (6400 bytes versus 5600 in logic)
;
#define BYTES_PER_TRACK		5600

		hlt
		nop
		nop
; Boot strap runs us from 0x0103
start:
		lda	tmpstack	; so we don't overwrite our stack
		xas
loop:

		lda	0x0010		; track 0 head 1 sector 0
		xaz
		lda	0x16E0		; DMA start address (after bootload)
		xay
;
;		Load tracks
;
load_loop:
		ldb	0x00A0
		sub	z,b
		bnz	not_last
		ldb	0xFFFF-5440		; end at EFFF
		bra	last
not_last:
		ldb	0xFFFF-BYTES_PER_TRACK
last:
		bsr	load_track
		ldb	5600
		add	b,y
		ldb	0x0010		; One head or cylinder
		add	b,z
		xfr	z,a
		ldb	0x00B0		; 
		sab
		bnl	load_loop
		jmp 	go

load_track:
muxwait:
		ldab	(0xF200)
		sra
		sra
		bnl	muxwait
		ldab	'*'
		stab	(0xF201)

		ldab	2
		stab	(0xF148)
waitseek:
		xfr 	z,a
		sta	(0xF141)
		bsr	wait
		dmamode	0
		dmaen
		stdma	y
		; One track worth
		stdmac	b
		clab
		stab	(0xF148)
wait:
		ldab	(0xF144)
		srrb	al
		bl	wait
		rsr
		.ds	8
tmpstack:
go:
