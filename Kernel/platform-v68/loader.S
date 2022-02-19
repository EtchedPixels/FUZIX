.mri	1
;
;	Simple bootstrap loader
;
;	Memory Map
;	$0000-$03FF	Vectors
;	$0400-$0FFF	Boot code
;	$1000-$11FF	Disk boot block loaded here
;	$..  -$1FFF	Stack
;	$2000-....	RAM for OS (can also recover rest)
;
;
	long   $15C0DE68
go:
;
;	On entry A6 is the PROM jump table vector. Preserve it
;
	movea	#initmsg,a3
	jsr	8(a6)		; string out
	moveq	#0,d0		; disk 0
	move.l	#1,d1		; block number (after partitions)
	move.b	#$80,d2		; count	(64K)
	move	#$2000,a0	; load address (after PROM stack)
diskloop:
	movem.l	d0-d2/a0,-(sp)
	jsr	24(a6)
	tst.b	d0
	bne	failload
	movem.l (sp)+,d0-d2/a0
	adda	#$200,a0
	addq	#1,d1
	sub.b	#1,d2
	bne diskloop

	move.l	$2000,d0
	cmp.l	#$1505C0DE,d0
	bne	notboot
	movea	#ok,a3
	jsr	8(a6)
	jmp	$2004

failload:
	movea	#failmsg,a3
fails:
	jsr	8(a6)
	jmp	(a6)

notboot:
	movea	#notbootmsg,a3
	bra	fails

ok:
	asciz	"OK\n"
notbootmsg:
	asciz	"Not a bootable image\n"
failmsg:
	asciz	"Disk error\n"
initmsg:
	asciz	"Loading..."
