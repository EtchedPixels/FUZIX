;
;	Standard common. Nothing special here except to remember that writes
;	to common space won't write through all banks
;
        .module commonmem

        .area _COMMONMEM

	.include "../../cpu-z80/std-commonmem.s"

