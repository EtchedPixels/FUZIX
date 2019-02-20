;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
;	As this is meant to be a simple reference port we use the standard
;	approach. The morbidly curious can read the TRS80 model 1 bank to
;	bank copier.
;
	.include "kernel.def"
	.include "../kernel-z80.def"

	.include "../lib/z80fixedbank.s"
