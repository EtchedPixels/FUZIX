;
;	Put the udata at the start of common. We have four 16K banks so we
; keep the non .common kernel elements below C000 and then keep bank 3 as a
; true common bank
;
        .module commonmem

        .area _COMMONMEM

	.include "../cpu-z80/std-commonmem.s"
