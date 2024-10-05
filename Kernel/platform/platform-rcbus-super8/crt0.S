#include "../../cpu-super8/kernel-super8.def"

		.code
.export init

	.word 0x5A38	; magic
	; Next 5 words and this get trampled by IRQ vectors

init:
        di
	srp #0x10
	ldw 216,#kstack_top

        call init_early

	; Shuffle up the segments we packed at the end and which
	; cover E000-FFFF areas. We put them in the upper 8K of
	; the data area in the load (C000-DFFF)
	ld r12,#0xC0
	clr r13
	ldw rr14,#__common
	ldw rr10,#__common_size
	decw rr14
copy_common:
	ldei r9,@rr12
	ldcpi @rr14,r9		; From data to code map
	decw rr10
	jr nz, copy_common

copy_commondata:
	; Currently no commondata - add if needed

	ldw rr12,#__bss
	ldw rr10,#__bss_size
	clr r0
	decw rr12
wipe:
	ldepi @rr12,r0
	decw rr10
	jr nz, wipe
	
        call init_hardware

        call _fuzix_main
        di
end:    jr end
