#include "kernel-armm0.def"

.code 16

.text
.align 4
.thumb_func
# On entry:
#   r0: entrypoint
.global doexec
doexec:
	ldr r1, =udata
	ldr r1, [r1, #U_DATA__U_SP_OFFSET]
	mov sp, r1
	isb
	bx r0

