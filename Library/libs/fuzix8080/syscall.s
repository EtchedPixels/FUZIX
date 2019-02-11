!
!	This uses the revised API that Z80 will move to (or at least
!	similarly). It's basically the same API with a 2 byte stack offset
!
.sect .text
.define __syscall

__syscall:
		rst 	6
		! returns in HL
		xchg				! into DE for the 8080 C ABI
		rnc				! ok return in DE
		xchg
		shld	_errno			! error path
		lxi	d,0xffff
		ret
