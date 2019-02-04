!
!	We will go to a sane API as Z80 does because Z80 can run 8080 apps
!	so they should change together. For now this is the same code as
!	the Z80 but in 8080 mnemonics
!
.sect .text
.define __syscall

__syscall:
		xthl
		xchg
		rst 	6
		xchg
		xthl
		xchg
		rnc				! ok
		shld	_errno			! error path
		lxi	h,0xffff
		ret
