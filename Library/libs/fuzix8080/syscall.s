!
!	This uses the revised API that Z80 now uses. We have to switch DE
!	and HL around a bit for 8080 because our C return is via DE not HL
!
.sect .text
.define __syscall

__syscall:
		call __text			! syscall stub is at text
						! start, set by kernel
		! returns in HL
		xchg				! into DE for the 8080 C ABI
		rnc				! ok return in DE
		xchg
		shld	_errno			! error path
		lxi	d,0xffff
		ret
