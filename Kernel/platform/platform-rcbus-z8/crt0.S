#include "../../cpu-z8/kernel-z8.def"


;
;	Loaded at 0x0100
;	Execution begins at 0x0102
;	Loaded as a direct image map
;
		.code
.export init

	.word 0x5A38	; magic

init:
        di
	srp #0x10
	ld 254,#>kstack_top
	ld 255,#<kstack_top

	; Must run before any C code to set up the fast pointers
	call __reginit

        call init_early

	; Wipe the BSS

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
