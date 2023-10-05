;
;	We have four independent 16K banks so we keep common high (and per
;	process). We avoid the last 16 bytes as on bank 3 thats magically
;	keyboard data.
;

        .module commonmem

        .area _COMMONMEM

	.include "../../cpu-z80/std-commonmem.s"

