;
;	Common on z80pack is at 0xF000 as defined by hardware.
;

        .module commonmem

        .area _COMMONMEM

	.include "../../cpu-r2k/std-commonmem.s"
