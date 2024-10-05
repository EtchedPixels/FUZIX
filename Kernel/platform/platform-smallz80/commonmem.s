;
;	The common memory area traditionally starts with the udata and the
;	interrupt stacks. As this is standard in almost all cases you can
;	just include the standard implementation.
;
        .module commonmem

        .area _COMMONMEM

	.include "../../cpu-z80/std-commonmem.s"

