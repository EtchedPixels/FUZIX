/*
 *	Boot off an RX0. 
 */

start:
	nop
	mov	$hello,r0
loop:
	movb	0177564,r2
	bpl	loop
	movb	(r0)+,r1
	beq 	done
	movb	r1,0177566
	br	loop
done:
	jsr	pc, donereq
	mov	$0x200,r0
	movb	$111, r1		// 111 blocks is our 56K
	mov	$0177170,r2		// controller base
	movb	$1,r3			// sector
	movb	$2,r4			// track
next:
	movb	$0x07,(r2)
	jsr	pc, txwait
	movb	r3,2(r2)
	jsr	pc, txwait
	movb	r4,2(r2)
	jsr	pc, donereq

	movb	$0x80,r5
	movb	$0x03,(r2)
load:
	tstb	(r2)
	bpl	load
	movb	2(r2),(r0)+
	dec	r5
	bne	load

	dec	r1
	beq	_finished
	inc	r3
	cmp	$27,r3
	bne	next
	movb	$1,r3
	inc	r4
	br	done

donereq:
	bis	$0x20,(r2)
	beq	donereq
	rts	pc

txwait:
	tstb	(r2)
	bpl	txwait
	rts	pc

_finished:
	jmp	0x200

hello:	.asciz "Loading Fuzix..."
