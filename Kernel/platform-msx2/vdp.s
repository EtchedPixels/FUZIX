            .module vdp

            .include "kernel.def"
            .include "../kernel.def"

	    .include "../dev/vdp2.s"

	    .area _COMMONMEM

;
;	FIXME: should use vdpport, but right now vdpport is in data not
;	common space.
;
platform_interrupt_all:
	    ld c, #0x99
	    in a, (c)
	    ret
