#include "numbers.h"

.area _CODE
.globl __syscall
.globl _@NAME@
_@NAME@:
	ld hl, #__syscall_@NAME@
	jp __syscall

