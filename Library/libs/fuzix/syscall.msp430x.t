#include "numbers.h"

.text
.globl _syscall
.globl @NAME@
@NAME@:
	mov #__syscall_@NAME@, r11
	br #_syscall


