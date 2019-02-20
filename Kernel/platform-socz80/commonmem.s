; 2013-12-18 William R Sowerbutts

        .module commonmem

        .include "kernel.def"
        .include "../kernel-z80.def"

        .area _COMMONMEM
        ; ------------------------------------------------------------------------------------------------
        ; 0xF800: start of common memory
        ; ------------------------------------------------------------------------------------------------

	.include "../cpu-z80/std-commonmem.s"

