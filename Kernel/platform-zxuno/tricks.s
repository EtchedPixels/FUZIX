
	.include "../lib/z80fixedbank-banked.s"

	.globl _portff
	.globl switch_bank
	.globl map_kernel_restore
	.globl ksave_map
	.globl current_map

	.globl bankfork

bankfork:
;
;	A = parent bank
;	C = child
;
;	1 main memory
;	2 ext
;	3 dock
;
;	We have three cases
;
;	1. 	Dock/Ext -> Main
;	2.	Main -> Dock/Ext
;	3.	Dock/Ext -> Dock/Ext
;
	ld b,a

	; Set up so we can map_kernel_restore to undo
	; the mess we create
	ld a,(current_map)
	ld (ksave_map),a

	bit 1,b		; parent is ext/dock (2 or 3)
	jr z, from_main	; must be bank 1 -> so parent is in main
	; From ext or dock to where ?
	bit 1,c		; is the child also in ext/dock
	jr z, to_main	; no - so we are copying from ext/doc to main
	; Ext to Dock or Dock to Ext
	; Use port 0xFF to do the work
	ld a,#0xF8
	out (0xF4),a		; Set up the MMU
	ld a,#0xFF
	ld (bankpatch1 + 1),a
	ld (bankpatch2 + 1),a
	ld a,(_portff)
	and #0x7F
	bit 0,c			; C = 2 (child is 2 and in ext)
	jr z, dock2ext
	or #0x80		; C = 3 (parent is 2 and in ext)
dock2ext:
	ld (cpatch0 + 1),a	; source
	xor #0x80
	ld (cpatch1 + 1), a	; dest
	jp do_copies

;
;	For a copy via main memory we can set the FF selection up once
;	and use F4 to switch from internal to external banks.
;
from_main:
	ld a,(_portff)
	and #0x7f
	bit 0,c			; child is which ?
	jr z, main2exit
	or #0x80		; dock to main
main2exit:
	out (0xFF),a		; select the right external bank
	xor a
	jr viamain
to_main:
	ld a,(_portff)
	and #0x7F
	bit 0,b			; parent is which ?
	jr nz, dock2main
	or #0x80		; exrom to main
dock2main:
	out (0xFF),a		; right bank
	ld a,#0xF8		; F4 port value to use
viamain:
	ld (cpatch0 + 1), a
	xor #0xF8
	ld (cpatch1 + 1), a
	ld a,#0xF4		; We will toggle 0xF4
	ld (bankpatch1 + 1) ,a
	ld (bankpatch2 + 1), a
	; We are going from a bank to main memory.
	ld a,#4
	call switch_bank	; we need page 4 visible
	jp do_copies

;
;	We patch this so it must be in writable memory
;
	.area _COMMONDATA

do_copies:
	;
	;	Set up ready for the copy
	;
	push ix
	ld (spcache),sp
	; Stack pointer at the target buffer
	ld sp,#PROGBASE	; Base of memory to fork
	; 10 outer loops - copy 40K
	ld a,#10
	ld (copyct),a
	xor a		; 256 inner loops of 16 (total 4K a loop)
copyloop:
	ex af,af'	; Save A as we need an A for ioports
cpatch0:
	ld a,#0		; parent bank (patched in for speed)
bankpatch1:
	out (0x00),a
	pop bc		; copy 16 bytes out of parent
	pop de
	pop hl
	exx
	pop bc
	pop de
	pop hl
	pop ix
	pop iy
	ld (sp_patch+1),sp
cpatch1:
	ld a,#0		; child bank (also patched in for speed)
bankpatch2:
	out (0x00),a
	push iy		; and put them back into the child
	push ix
	push hl
	push de
	push bc
	exx
	push hl
	push de
	push bc
	ex af,af'	; Get counter back
	dec a
	jp z, setdone	; 256 loops ?
copy_cont:
sp_patch:
	ld sp,#0
	jp copyloop

	.area _COMMONMEM
;
;	This outer loop only runs 10 times so isn't quite so performance
;	critical
;
setdone:
	ld hl,#copyct
	dec (hl)
	jr z, copy_over
	xor a
	jp copy_cont
copy_over:
	;
	;	Get the stack back
	;
	ld sp,(spcache)
	;
	;	And the correct kernel bank.
	;
	pop ix
	jp map_kernel_restore

	.area _COMMONDATA

spcache:
	.word 0
copyct:
	.byte 0
