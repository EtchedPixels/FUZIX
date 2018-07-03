	.module stringy

	.globl	_tape_op
	.globl	_tape_err

        .include "kernel.def"
        .include "../kernel.def"

	.area _COMMONMEM

	.globl _tape_op

	.globl go_slow
	.globl go_fast

_tape_op:
	call go_slow
	pop bc
	pop de
	push de		; file ID in D op in E
	push bc
	ld hl,(U_DATA__U_BASE)
	ld bc,(U_DATA__U_COUNT)
	xor a
	ld (_tape_err),a
	ld a,e
	or a
	jr z, write_op
	dec a
	jr z, read_op
	dec a
	jr z, closew_op
	dec a
	jr z, rewind_op
	dec a
	jr z, seekf_op
	dec a
	jr z, select_op
	dec a
	jr z, erase_op
	ld hl,#0xFFFF
	call go_fast
	ret
find_op:
	ld a,d
	call 0x300F
	jr tape_error
select_op:
	ld a,d			; not a file ID but a drive ID this time
	call 0x3012
	jr tape_error
seekf_op:
	ld a,d
	call 0x300F		; find file A
tape_error:
	push af
	call go_fast
	pop af
	ld hl,#0		; return 0 if good or -1 if bad and save err
	ret z
	dec hl
	ld (_tape_err),a
	ret
closew_op:
	ld a,d
	ld bc,#0
	call 0x3027		; Write an EOF and the marker for the next
	jr tape_error		; file ID
rewind_op:
	call 0x3000		; Go back to start of tape (does not find
	jr tape_error		; file 0 itself)
read_op:
	call 0x3003
	push af
	call go_fast
	pop af
	ld h,b
	ld l,c			; actual bytes fetched
	ret z
rwerr:
	ld (_tape_err),a
	ld hl,#0xffff
	ret
write_op:
	call 0x3006
	push af
	call go_fast
	pop af
	ld hl,(U_DATA__U_COUNT)
	ret z
	jr rwerr
erase_op:
	ld a,d
	call 0x3024
	jr tape_error
