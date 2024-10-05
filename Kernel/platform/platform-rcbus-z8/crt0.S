#include "../../cpu-z8/kernel-z8.def"

		.code
.export init

	.word 0x5A38	; magic
	; Next 5 words and this get trampled by IRQ vectors

init:
        di
	srp #0x10
	ld 254,#>kstack_top
	ld 255,#<kstack_top

        call init_early

	; Shuffle up the segments we packed at the end and which
	; cover E000-FFFF areas. We put them in the upper 8K of
	; the data area in the load (C000-DFFF)
	ld r12,#0xC0
	clr r13
	ld r14,#>__common
	ld r15,#<__common
	ld r10,#>__common_size
	ld r11,#<__common_size
copy_common:
	lde r9,@rr12
	ldc @rr14,r9		; From data to code map
	incw rr12
	incw rr14
	decw rr10
	jr nz, copy_common

copy_commondata:
	; Currently no commondata - add if needed

	ld r12,#>__bss
	ld r13,#<__bss
	ld r10,#>__bss_size
	ld r11,#<__bss_size
	clr r0
wipe:
	lde @rr12,r0
	incw rr12
	decw rr10
	jr nz, wipe
	
        call init_hardware

        call _fuzix_main
        di
end:    stop
        jp end
