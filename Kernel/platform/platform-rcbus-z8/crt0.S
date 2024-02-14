#include "../../cpu-z8/kernel-z8.def"

		.code

.export init

	.word 0x5A38

init:
        di
	srp #10
	ld 254,#>kstack_top
	ld 255,#<kstack_top

        call init_early

	; Common is packed in the BSS space

	ld r14,#>__common
	ld r15,#<__common
	ld r12,#>__bss
	ld r13,#<__bss
	ld r10,#>__common_size
	ld r11,#<__common_size

	; Copy it high
nextbyte:
	lde r0,@rr12
	lde @rr14,r0
	incw rr12
	incw rr14
	decw rr10
	jr nz, nextbyte

	; The discard follows the common
	ld r14,#>__discard
	ld r15,#<__discard
	ld r10,#>__discard_size
	ld r11,#<__discard_size
	; Go from the end as we want to copy backwards in case of
	; an overlap
	add r15,r11
	adc r14,r10
	add r13,r11
	adc r12,r10

nextbyted:
	decw rr12
	decw rr14
	lde r0,@rr12
	lde @rr14,r0
	decw rr10
	jr nz, nextbyted

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
