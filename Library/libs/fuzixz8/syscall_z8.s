	.export __syscall
	.code

__syscall:
	call __text
	jr nc,noerr
	ld r14,#>_errno
	ld r15,#<_errno
	lde @rr14,r0
	incw rr14
	lde @rr14,r1
	ld r2,#255
	ld r3,r2
noerr:
	ret
