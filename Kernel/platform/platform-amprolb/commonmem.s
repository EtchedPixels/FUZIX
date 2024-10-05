;
;	Common is placed at 0xF000 by fuzix.lnk
;

        .module commonmem

        .area _COMMONMEM

	.include "../../cpu-z80/std-commonmem.s"
