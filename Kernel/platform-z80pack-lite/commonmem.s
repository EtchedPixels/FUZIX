;
;	Common on z80pack-lite is at 0xF900 to give all we can to user/swap. 
;

        .module commonmem

        .area _COMMONMEM

	.include "../cpu-z80/std-commonmem.s"
