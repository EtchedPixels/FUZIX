	.export __syscall
	.code

__syscall:
	call __text
	jr nc,noerr
	ldw rr14,#_errno
	lde @rr14,r0
	ldepi @rr14,r1
	ld r2,#255
	ld r3,r2
noerr:
	ret
