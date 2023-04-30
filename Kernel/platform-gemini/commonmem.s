;
;	We only have a small amount of true (ie writable) common space. We
;	don't really want stacks and stuff in it (it's contended by video)
;	but for now this will get us going
;
        .module commonmem

        .area _COMMONDATA

	.include "../cpu-z80/std-commonmem.s"

