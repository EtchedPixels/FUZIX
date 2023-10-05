;
; Multiple app sizes and the fact the kernel and apps share the same banks
; means we need to put this somewhere low
;
        .module commonmem
        .area _COMMONDATA

	.include "../../cpu-z80/std-commonmem.s"
