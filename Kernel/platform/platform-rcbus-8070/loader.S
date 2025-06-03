;
;	We are loaded at 0x0100
;
;	We load an image from 0200-FDFF
;	and then jump to 0202 if the marker is right
;
	.code

	.byte	0x80
	.byte	0x70
start:
	ld	p3,=0xFE78
	ld	ea,=0x2322
	st	ea,2,p3		; set top pages to 0x22 0x23

	; Our map is now 32 33 34 35

	ld	p1,=0x0100	; Stack out of the way

	ld	p3,=0x0200

	ld	a,=0x01	; 0 is the partition/boot block
	st	a,:sector

	ld	p2,=0xFE10

	jsr	waitready

	ld	a,=0xE0
	st	a,6,p2
dread:
	jsr	waitready
	ld	a,='.'
	jsr	outchar
	ild	a,:sector
	sub	a,=0x7D
	bz	load_done
	ld	a,:sector
	st	a,3,p2		; sector LBA
	ld	a,=1
	st	a,2,p2		; num sectors (drives may clear this each I/O)
	jsr	waitready
	ld	a,=0x20
	st	a,7,p2		; read command

	jsr	waitdrq

	ld	a,=0
	st	a,:count
bytes:
	ld	a,0,p2
	st	a,@1,p3
	dld	a,:count
	bnz	bytes
	bra	dread

load_done:
	ld	p3,=0x0200
	ld	ea,0,p3
	sub	ea,=0x8070
	or	a,e
	bnz	bad_load

	ld	p2,=running
	jsr	outstring
	jmp	0x0102		; jump to byte following

bad_load:
	ld	p2,=badimg
	jsr	outstring
stop:
	bra	stop

waitready:
	ld	a,7,p2
	and	a,=0x40
	bz	waitready
	ret

waitdrq:
	ld	a,7,p2
	and	a,=0x09
	bz	waitdrq
	and	a,=0x01
	bz	wait_drq_done
	ld	a,1,p2
	jsr	outcharhex
	bra	bad_load
wait_drq_done:
outdone1:
	ret

outstring:
	ld	a,@1,p2
	bz	outdone1
	jsr	outchar
	bra	outstring

outcharhex:
	push	a
	sr	a
	sr	a
	sr	a
	sr	a
	jsr	outcharhex1
	ld	a,e
outcharhex1:
	and	a,=0x0F
	add	a,=0x30
	bnd	hexit
outchar:
	push	p2
	ld	p2,=0xFEC0
	ld	e,a
outcharw:
	ld	a,5,p2
	and	a,=0x20
	bz	outcharw
	ld	a,e
	st	a,0,p2
	pop	p2
	ret
hexit:
	add	a,=7
	bra	outchar

badimg:
	.byte	13,10
	.ascii	"Image not bootable."
running:
	.byte	13,10,0

	.dp

sector:
	.byte	0
count:
	.byte	0
