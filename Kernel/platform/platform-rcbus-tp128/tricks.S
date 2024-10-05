;
;	For simple banked systems there is a standard implementation. The
;	only reason to do otherwise is for speed. A custom bank logic aware
;	bank to bank copier will give vastly better fork() performance.
;
#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

;
;	All of the fixed bank support is available as a library routine,
;	however it is a performance sensitive area. Start with
;
#include "../../lib/z80ufixedbank.S"
;
;	As well as using "usermem_std-z80.rel" in your link file for the
;	userspace access operations.
;
;	We can also use the standard fast user copiers because all the
;	kernel objects we need to copy to/from userspace exist in the
;	user mapping.
;
#include "../../lib/z80uuser1.s"

