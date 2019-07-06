#

#include "../kernel-8080.def"
#include "../lib/8085fixedbank.s"

.sect .common

.define bankfork

!
!	A is the parent base bank C is the child base bank
!
bankfork:
	di
	mov b,a
	! Do 48K of copying (C000-FFFF is the common bank)
	call copy16
	call copy16
	call copy16
	! Fix up the mess
	jmp map_kernel

copy16:
	mov a,b
	out 0x79		! 0x4000 is now the parent 16K
	mov a,c
	out 0x7A		! 0x8000 is the child 16K

	push b
	call fastcopy
	pop b

	inr b
	inr c
	ret

!
!	The basic idea is to work in words as best we can
!
!	lhlx loads HL from (DE), and the push then both
!	stores the word in the right place and adjusts sp.
!	We adjust d and run an unrolled loop
!
fastcopy:
	lxi h,0
	dad sp
	shld tmpsp+1
	lxi b,0x07FF		! we need to wrap for the jnk
	lxi d,0x7FFE
	lxi sp,0xC000
copywords:
	! 16 bytes a loop
	lhlx
	push h
	dcx d
	dcx d
	lhlx
	push h
	dcx d
	dcx d
	lhlx
	push h
	dcx d
	dcx d
	lhlx
	push h
	dcx d
	dcx d
	dcx b
	lhlx
	push h
	dcx d
	dcx d
	lhlx
	push h
	dcx d
	dcx d
	lhlx
	push h
	dcx d
	dcx d
	lhlx
	push h
	dcx d
	dcx d
	dcx b
	jnk copywords
tmpsp:
	lxi sp,0	! patched
	ret
