;
;	We are loaded at 0xFC00. We unmap the ROM, then go

	.dp

dptr:	.word 0
sec:	.byte 0
tmp:	.byte 0

	.abs

	.org 0xFC00

loader:	; loaded at FC00 with stack up top (FFFF)
	ld	ea,=0
	st	ea,:dptr
	st	a,:sec		; we will inc before use and start at 1
	ld	p3,=0xFE00
	ld	p2,=0xFEC0
	ld	a,=0x20
	st	a,0x78,p3	; Set low 16K to RAM 0 so we are now all RAM

block:	ild	a,:sec
	add	a,=1
	bp	next		; sectors 1 to 0x7E
	jmp	0x0001
next:
	jsr	sector
nch:	ld	a,5,p2
	and	a,=0x20
	bz	nch
	ld	a,='.'
	st	a,0,p2
	bra	block

sector:
	push	p2
	ld	a,=1
	st	a,0x12,p3	; one sector
	ld	a,:sec
	st	a,0x13,p3	; sector count low
	ld	ea,=0
	st	ea,0x14,p3	; clear high bits
	ld	a,=0x20
	st	a,0x17,p3
	jsr	waitdrq

	ld	ea,:dptr
	ld	p2,ea
	ld	a,=0
	st	a,:tmp
loop:
	ld	a,0x10,p3
	st	a,@1,p2
	ld	a,0x10,p3
	st	a,@1,p2
	dld	a,:tmp
	bnz	loop

	ld	ea,p2
	st	ea,:dptr
	pop	p2
	ret

waitready:
	ld	a,0x17,p3
	and	a,=0x40
	bz	waitready
	ret

waitdrq:
	ld	a,0x17,p3
	and	a,=0x09
	bz	waitdrq
	and	a,=0x01
	bnz	fail
	ret
fail:
	jsr	outs
	.ascii	"Disk Error"
	.byte	13,10,0
dead:	bra	dead

outs:
	push	p2
	push	p3
	ld	ea,4,p1		; return addr
	ld	p3,ea
	ld	p2,=0xFEC0
nextch:
	ld	a,5,p2
	and	a,=0x20
	bz	nextch
	ld	a,@1,p3
	bz	end
	st	a,0,p2
	bra	nextch
end:
	ld	ea,p3
	st	ea,4,p1
	pop	p3
	pop	p2
	ret
