	.module devrd_hw

	; imported symbols - from zeta-v2.s
	.globl map_table

	; exported symbols (used by devrd.c)
	.globl _page_copy
	.globl _src_page, _src_offset, _dst_page, _dst_offset, _cpy_count

	.include "kernel.def"

	.area _COMMONMEM

;=========================================================================
; _page_copy - Copy data from one physical page to another
; Inputs:
;   _src_page - page number of the source page (uint8_t)
;   _src_offset - offset in the source page (uint16_t)
;   _dst_page - page number of the destination page (uint8_t)
;   _dst_offset - offset in the destination page (uint16_t)
;   _cpy_count - number of bytes to copy (uint16_t)
;   map_table - current page register values, used to restore paging registers
; Outputs:
;   Data copied
;   Destroys AF, BC, DE, HL
;=========================================================================
_page_copy:
	ld a,(_src_page)
	out (MPGSEL_1),a		; map source page to bank #1
	ld a,(_dst_page)
	out (MPGSEL_2),a		; map destination page to bank #2
	ld hl,(_src_offset)		; load offset in source page
	ld a,#0x40			; add bank #1 offset - 0x4000
	add h				; to the source offset
	ld h,a
	ld de,(_dst_offset)
	ld a,#0x80			; add bank #2 offset - 0x8000
	add d				; to the destination offset
	ld d,a
	ld bc,(_cpy_count)		; bytes to copy
	ldir				; do the copy
	ld a,(map_table+1)
	out (MPGSEL_1),a		; restore the mapping of bank #1
	ld a,(map_table+2)
	out (MPGSEL_2),a		; restore the mapping of bank #2
	ret

; variables
_src_page:
	.db	0
_dst_page:
	.db	0
_src_offset:
	.dw	0
_dst_offset:
	.dw	0
_cpy_count:
	.dw	0
;=========================================================================
