;
; We need to put commonmem above 0xC000 for multi tasking.
;
        .module commonmem
        .area _COMMONMEM

	.include "../cpu-z80/std-commonmem.s"
