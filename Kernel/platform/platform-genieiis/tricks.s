
	.include "kernel.def"
	.include "../kernel-z80.def"

	.include "../lib/z80fixedbank.s"
	; We keep all user copy in / out data above C000 so can use the fast
	; helpers
	.include "../lib/z80user1.s"

	; TODO: custom bank copier