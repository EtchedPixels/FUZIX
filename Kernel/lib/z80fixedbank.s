
	.include "../lib/z80fixedbank-core.s"
;
;	This is related so we will keep it here. Copy the process memory
;	for a fork. a is the page base of the parent, c of the child
;
;	Assumption - fits into a fixed number of whole 256 byte blocks
;

	.area _COMMONMEM

bankfork:
	ld b, #(U_DATA_STASH - PROGBASE)/256
	ld hl, #PROGBASE	; base of memory to fork (vectors included)
bankfork_1:
	push bc			; Save our counter and also child offset
	push hl
	call map_process_a
	ld de, #bouncebuffer
	ld bc, #256
	ldir			; copy into the bounce buffer
	pop de			; recover source of copy to bounce
				; as destination in new bank
	pop bc			; recover child page number
	push bc
	ld b, a			; save the parent bank id
	ld a, c			; switch to the child
	call map_process_a
	push bc			; save the bank pointers
	ld hl, #bouncebuffer
	ld bc, #256
	ldir			; copy into the child
	pop bc			; recover the bank pointers
	ex de, hl		; destination is now source for next bank
	ld a, b			; parent bank is wanted in a
	pop bc
	djnz bankfork_1		; rinse, repeat
	ret
