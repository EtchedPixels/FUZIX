;
;	Do this in assembler so we can keep the vtbackbuf banked
;
		.module vtswap

		.area _CODE2

		.globl _vtswap
		.globl _vtbackbuf

_vtbackbuf:
		.ds 1024

_vtswap:
                ld hl, #0xf800
                ld de, #_vtbackbuf
                ld bc, #1024	; 64 * 16
        exchit:
                push bc
                ld a, (de)	; Could be optimised but its only 1K
                ld c, (hl)	; Probably worth doing eventuallly
                ex de, hl
                ld (hl), c
                ld (de), a
                inc hl
                inc de
                pop bc
                dec bc
                ld a, b
                or c
                jr nz, exchit
                ret
