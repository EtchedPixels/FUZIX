;
;	We have no real common on the TRS80so just tuck it up at the top of
;	memory leaving room for the keyboard and video (3K)
;
        .module commonmem

        .area _COMMONMEM

	.include "../cpu-z80/std-commonmem.s"

