;
;	Common is at 0xC000. We don't actually have any choice about that
;	the platform is wired this way.
;

        .module commonmem

        .area _COMMONMEM

	.include "../cpu-z80/std-commonmem.s"
