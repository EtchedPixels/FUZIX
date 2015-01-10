	.include "platform/kernel.def"
        .include "kernel02.def"

	.export __uget, __ugetc, __ugetw, __ugets
	.export __uput, __uputc, __uputw, __uzero
;
;	TO DO: Possibly easier to write them in C with just the _c helpers
;	but for speed would be best the block ones are asm.
;
__uget:		rts
__ugetc:	rts
__ugetw:	rts
__ugets:	rts
__uput:		rts
__uputc:	rts
__uputw:	rts
__uzero:	rts
