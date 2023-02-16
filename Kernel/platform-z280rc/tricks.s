;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
	.include "kernel.def"
	.include "../kernel-z80.def"

	.include "../lib/z80fixedbank.s"
