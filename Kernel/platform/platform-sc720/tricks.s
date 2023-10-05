;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

;
;	All of the fixed bank support is available as a library routine,
;	however it is a performance sensitive area. Start with
;
	.include "../../lib/z80fixedbank.s"
;
;	As well as using "usermem_std-z80.rel" in your link file for the
;	userspace access operations.
;
;	We can also use the standard fast user copiers because all the
;	kernel objects we need to copy to/from userspace exist in the
;	user mapping.
;
	.include "../../lib/z80user1.s"

